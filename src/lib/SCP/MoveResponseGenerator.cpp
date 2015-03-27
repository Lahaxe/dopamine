/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "ConverterBSON/BSONToDataSet.h"
#include "ConverterBSON/DataSetToBSON.h"
#include "ConverterBSON/TagMatch.h"
#include "core/ConfigurationPACS.h"
#include "core/ExceptionPACS.h"
#include "core/LoggerPACS.h"
#include "core/NetworkPACS.h"
#include "MoveResponseGenerator.h"

namespace dopamine
{
    
/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param progress: progress state (in)
 * @param request: original store request (in)
 */
static void moveSubProcessCallback(void*, T_DIMSE_StoreProgress * progress,
                                   T_DIMSE_C_StoreRQ*)
{
    // Nothing to do
}
    
MoveResponseGenerator
::MoveResponseGenerator(MoveSCP * scp, std::string const & ouraetitle):
    ResponseGenerator(scp, ouraetitle) // base class initialisation
{
    _origAETitle[0] = '\0';
}

MoveResponseGenerator
::~MoveResponseGenerator()
{
    //Nothing to do
}

void 
MoveResponseGenerator
::callBackHandler(
        /* in */
        OFBool cancelled, T_DIMSE_C_MoveRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_MoveRSP* response, DcmDataset** stDetail,
        DcmDataset** responseIdentifiers)
{
    if (responseCount == 1)
    {
        // Look for user authorization
        if ( !NetworkPACS::get_instance().check_authorization(
                 this->_scp->get_association()->params->DULparams.reqUserIdentNeg,
                 Service_Retrieve) )
        {
            loggerWarning() << "User not allowed to perform MOVE";

            this->_status = STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches;
            response->DimseStatus = STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches;

            this->createStatusDetail(STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches, DCM_UndefinedTagKey,
                                     OFString("User not allowed to perform MOVE"), stDetail);

            return;
        }

        _origMsgID = request->MessageID;
        
        /* Start the database search */

        mongo::BSONObj constraint =
                NetworkPACS::get_instance().get_constraint_for_user(
                    this->_scp->get_association()->params->DULparams.reqUserIdentNeg,
                    Service_Retrieve);

        // Convert the dataset to BSON, excluding Query/Retrieve Level.
        DataSetToBSON dataset_to_bson;

        dataset_to_bson.get_filters().push_back(
            std::make_pair(TagMatch::New(DCM_QueryRetrieveLevel),
                           DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.get_filters().push_back(
            std::make_pair(TagMatch::New(DCM_SpecificCharacterSet),
                           DataSetToBSON::FilterAction::EXCLUDE));
        dataset_to_bson.set_default_filter(DataSetToBSON::FilterAction::INCLUDE);

        mongo::BSONObjBuilder query_builder;
        dataset_to_bson(requestIdentifiers, query_builder);
        mongo::BSONObj const query_dataset = query_builder.obj();

        // Build the MongoDB query and query fields from the query dataset.
        mongo::BSONObjBuilder db_query;
        mongo::BSONObjBuilder fields_builder;
        for(mongo::BSONObj::iterator it=query_dataset.begin(); it.more();)
        {
            mongo::BSONElement const element = it.next();
            std::vector<mongo::BSONElement> const array = element.Array();

            std::string const vr = array[0].String();
            mongo::BSONElement const & value = array[1];
            Match::Type const match_type = this->_get_match_type(vr, value);

            DicomQueryToMongoQuery function = this->_get_query_conversion(match_type);
            // Match the array element containing the value
            (this->*function)(std::string(element.fieldName())+".1", vr, 
                                          value, db_query);
        }
        
        // retrieve 'location' field
        fields_builder << "location" << 1;

        // Always include Specific Character Set in results.
        if(!fields_builder.hasField("00080005"))
        {
            fields_builder << "00080005" << 1;
        }

        // Always include the keys for the query level and its higher levels
        OFString ofstring;
        OFCondition condition = requestIdentifiers->findAndGetOFString(DCM_QueryRetrieveLevel,
                                                                       ofstring);
        if (condition.bad())
        {
            dopamine::loggerError() << "Cannot find DCM_QueryRetrieveLevel: "
                                    << condition .text();

            this->_status = STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass;
            response->DimseStatus = STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass;

            this->createStatusDetail(STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass,
                                     DCM_QueryRetrieveLevel, OFString(condition.text()), stDetail);
            return;
        }

        this->_query_retrieve_level = std::string(ofstring.c_str());
        if(!fields_builder.hasField("00100020"))
        {
            fields_builder << "00100020" << 1;
        }
        if((this->_query_retrieve_level=="STUDY" ||
            this->_query_retrieve_level=="SERIES" ||
            this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000d"))
        {
            fields_builder << "0020000d" << 1;
        }
        if((this->_query_retrieve_level=="SERIES" ||
            this->_query_retrieve_level=="IMAGE") && !fields_builder.hasField("0020000e"))
        {
            fields_builder << "0020000e" << 1;
        }
        if(this->_query_retrieve_level=="IMAGE" && !fields_builder.hasField("00080018"))
        {
            fields_builder << "00080018" << 1;
        }

        // Handle reduce-related attributes
        std::string reduce_function;
        mongo::BSONObjBuilder initial_builder;

        // Number of XXX Related Instances (0020,120X)
        if(query_dataset.hasField("00201204"))
        {
            this->_instance_count_tag = DCM_NumberOfPatientRelatedInstances;
        }
        else if(query_dataset.hasField("00201208"))
        {
            this->_instance_count_tag = DCM_NumberOfStudyRelatedInstances;
        }
        else if(query_dataset.hasField("00201209"))
        {
            this->_instance_count_tag = DCM_NumberOfSeriesRelatedInstances;
        }
        else
        {
            this->_instance_count_tag = DCM_UndefinedTagKey;
        }
        if (this->_instance_count_tag != DCM_UndefinedTagKey)
        {
            reduce_function += "result.instance_count+=1;";
            initial_builder << "instance_count" << 0;
        }

        mongo::BSONArrayBuilder finalquerybuilder;
        finalquerybuilder << constraint << db_query.obj();
        mongo::BSONObjBuilder finalquery;
        finalquery << "$and" << finalquerybuilder.arr();

        // Format the reduce function
        reduce_function = "function(current, result) { " + reduce_function + " }";

        // Perform the DB query.
        mongo::BSONObj const fields = fields_builder.obj();
        mongo::BSONObj group_command = BSON("group" << BSON(
            "ns" << "datasets" << "key" << fields << "cond" << finalquery.obj() <<
            "$reduce" << reduce_function << "initial" << initial_builder.obj()
        ));
        
        NetworkPACS::get_instance().get_connection().runCommand
            (NetworkPACS::get_instance().get_db_name(),
                group_command, this->_info, 0);
                
        this->_results = this->_info["retval"].Array();
        this->_index = 0;

        this->_status = STATUS_Pending;
        
        // Build a new association to the move destination
        condition = this->buildSubAssociation(request, stDetail);
        if (condition.bad())
        {
            dopamine::loggerError() << "Cannot create sub association: "
                                    << condition.text();
        }
    } // if (responseCount == 1)
    
    /* only cancel if we have pending responses */
    if (cancelled && this->_status == STATUS_Pending)
    {
        this->cancel();
    }
    
    /* Process next result */
    if (this->_status == STATUS_Pending)
    {
        this->_priority = request->Priority;
        this->next(responseIdentifiers, stDetail);
    }
    
    /* set response status */
    response->DimseStatus = this->_status;
    if (this->_status == STATUS_Pending || this->_status == STATUS_Success)
    {
        (*stDetail) = NULL;
    }
}

void 
MoveResponseGenerator
::next(DcmDataset ** responseIdentifiers, DcmDataset **details)
{
    if(this->_index == this->_results.size())
    {
        // We're done.
        this->_status = STATUS_Success;

        OFCondition cond = ASC_releaseAssociation(this->_subAssociation);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Release Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
        cond = ASC_destroyAssociation(&this->_subAssociation);
        if (cond.bad())
        {
            OFString temp_str;
            dopamine::loggerError() << "Cannot Destroy Association: "
                                         << DimseCondition::dump(temp_str, cond);
        }
    }
    else
    {
        mongo::BSONObj item = this->_results[this->_index].Obj();
        
        if ( ! item.hasField("location"))
        {
            dopamine::loggerError() << "Unable to retrieve location field";

            this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(EC_CorruptedData.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        
        std::string const path = item.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition result = fileformat.loadFile(path.c_str());
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot load dataset " << path << " : "
                                    << result.text();

            this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        DcmDataset* dataset = fileformat.getAndRemoveDataset();
        
        OFString sopclassuid;
        result = dataset->findAndGetOFString(DCM_SOPClassUID,
                                                         sopclassuid);
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot retrieve SOPClassUID in dataset: "
                                    << result.text();

            this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                     DCM_SOPClassUID, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        OFString sopinstanceuid;
        result = dataset->findAndGetOFString(DCM_SOPInstanceUID, 
                                             sopinstanceuid);
        if (result.bad())
        {
            dopamine::loggerError() << "Cannot retrieve SOPInstanceUID in dataset: "
                                    << result.text();

            this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                     DCM_SOPInstanceUID, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;
            return;
        }
        
        // Perform sub operation
        result = this->performMoveSubOperation(sopclassuid.c_str(), 
                                               sopinstanceuid.c_str(), 
                                               dataset);
        if (result.bad())
        {
            dopamine::loggerError() << "Move Sub-Op Failed: " << result.text();

            this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                     DCM_UndefinedTagKey, OFString(result.text()), details);

            this->_status = STATUS_MOVE_Failed_UnableToProcess;

            return;
        }
            
        ++this->_index;

        this->_status = STATUS_Pending;
    }
}

OFCondition 
MoveResponseGenerator
::performMoveSubOperation(const char* sopClassUID, 
                          const char* sopInstanceUID,
                          DcmDataset* dataset)
{
    /* which presentation context should be used */
    T_ASC_PresentationContextID presID;
    presID = ASC_findAcceptedPresentationContextID(this->_subAssociation, 
                                                   sopClassUID);
    if (presID == 0)
    {
        dopamine::loggerError() << "No presentation context for: "
                                << dcmSOPClassUIDToModality(sopClassUID, "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    
    DIC_US msgID = this->_subAssociation->nextMsgID++;
    
    T_DIMSE_C_StoreRQ req;
    req.MessageID = msgID;
    strcpy(req.AffectedSOPClassUID, sopClassUID);
    strcpy(req.AffectedSOPInstanceUID, sopInstanceUID);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = this->_priority;
    req.opts = O_STORE_MOVEORIGINATORAETITLE | O_STORE_MOVEORIGINATORID;
    strcpy(req.MoveOriginatorApplicationEntityTitle, this->_origAETitle);
    req.MoveOriginatorID = this->_origMsgID;
    
    dopamine::loggerInfo() << "Store SCU RQ: MsgID " << msgID;
    
    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    return DIMSE_storeUser(this->_subAssociation, presID, &req, NULL,
                           dataset, moveSubProcessCallback, this,
                           DIMSE_BLOCKING, 0, &rsp, &stdetail,
                           &cancelParameters);
}

OFCondition 
MoveResponseGenerator
::buildSubAssociation(T_DIMSE_C_MoveRQ* request, DcmDataset **details)
{
    DIC_AE dstAETitle;
    dstAETitle[0] = '\0';
    
    strcpy(dstAETitle, request->MoveDestination);
    
    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_scp->get_association()->params, 
                    this->_origAETitle, aeTitle, NULL);
    
    std::string dstHostNamePort;
    if (!ConfigurationPACS::get_instance().peerForAETitle(std::string(request->MoveDestination), 
                                                          dstHostNamePort))
    {
        dopamine::loggerError() << "Invalid Peer for move operation";

        this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                 DCM_UndefinedTagKey, OFString(EC_IllegalParameter.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return EC_IllegalParameter;
    }

    T_ASC_Parameters* params;
    OFCondition result = ASC_createAssociationParameters(&params, 
                                                         ASC_DEFAULTMAXPDU);
    if (result.bad())
    {
        this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                 DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }
    
    DIC_NODENAME localHostName;
    gethostname(localHostName, sizeof(localHostName) - 1);
    
    ASC_setPresentationAddresses(params, localHostName, dstHostNamePort.c_str());
    
    ASC_setAPTitles(params, this->_ourAETitle.c_str(), dstAETitle, NULL);
    
    result = this->addAllStoragePresentationContext(params);
    if (result.bad())
    {
        this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                 DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }
    
    // Create Association
    result = ASC_requestAssociation(NetworkPACS::get_instance().get_network(), 
                                    params, &this->_subAssociation);
    
    if(result.bad())
    {
        OFString empty;
        
        if(result == DUL_ASSOCIATIONREJECTED)
        {
            T_ASC_RejectParameters rej;
            ASC_getRejectParameters(params, &rej);
            
            dopamine::loggerError() << ASC_printRejectParameters(empty, &rej).c_str();
        } 
        else 
        {
            dopamine::loggerError() << DimseCondition::dump(empty, result).c_str();
        }

        this->createStatusDetail(STATUS_MOVE_Failed_UnableToProcess,
                                 DCM_UndefinedTagKey, OFString(result.text()), details);

        this->_status = STATUS_MOVE_Failed_UnableToProcess;
        return result;
    }

    return result;
}

OFCondition 
MoveResponseGenerator
::addAllStoragePresentationContext(T_ASC_Parameters* params)
{
    const char* transferSyntaxes[] = { NULL, NULL, NULL, NULL };
    int numTransferSyntaxes = 0;
    if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
    {
      transferSyntaxes[0] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_BigEndianExplicitTransferSyntax;
    } else {
      transferSyntaxes[0] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[1] = UID_LittleEndianExplicitTransferSyntax;
    }
    transferSyntaxes[2] = UID_LittleEndianImplicitTransferSyntax;
    numTransferSyntaxes = 3;
    
    OFCondition cond = EC_Normal;
    int pid = 1;
    for (int i = 0; i < numberOfDcmLongSCUStorageSOPClassUIDs && cond.good(); i++)
    {
        cond = ASC_addPresentationContext(params, pid, dcmLongSCUStorageSOPClassUIDs[i], 
                                          transferSyntaxes, numTransferSyntaxes);
        pid += 2;
    }
    
    return cond;
}
    
} // namespace dopamine
