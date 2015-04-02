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
#include "core/LoggerPACS.h"
#include "GetResponseGenerator.h"
#include "services/ServicesTools.h"

namespace dopamine
{

namespace services
{
    
/**
 * Callback handler called by the DIMSE_storeProvider callback function
 * @param progress: progress state (in)
 * @param request: original store request (in)
 */
static void getSubProcessCallback(void*, T_DIMSE_StoreProgress * progress,
                                  T_DIMSE_C_StoreRQ* request)
{
    // Nothing to do
}
    
GetResponseGenerator
::GetResponseGenerator(T_ASC_Association *request_association):
    ResponseGenerator(request_association, Service_Retrieve), // base class initialisation
    _priority(DIMSE_PRIORITY_MEDIUM)
{
    // Nothing to do
}

GetResponseGenerator
::~GetResponseGenerator()
{
    // Nothing to do
}
    
void 
GetResponseGenerator
::process(
    /* in */
    OFBool cancelled, T_DIMSE_C_GetRQ* request,
    DcmDataset* requestIdentifiers, int responseCount,
    /* out */
    T_DIMSE_C_GetRSP* response, DcmDataset** stDetail,
    DcmDataset** responseIdentifiers)
{
    if (responseCount == 1)
    {
        this->_status = this->set_query(requestIdentifiers);

        if (this->_status != STATUS_Success && this->_status != STATUS_Pending)
        {
            response->DimseStatus = this->_status;
            createStatusDetail(this->_status, DCM_UndefinedTagKey,
                               OFString("An error occured while processing Get operation"),
                               stDetail);
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
GetResponseGenerator
::next(DcmDataset ** responseIdentifiers, DcmDataset **details)
{
    if(!this->_cursor->more())
    {
        // We're done.
        this->_status = STATUS_Success;
    }
    else
    {
        mongo::BSONObj item = this->_cursor->next();
        
        if ( ! item.hasField("location"))
        {
            dopamine::loggerError() << "Unable to retrieve location field.";

            createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(EC_CorruptedData.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        
        std::string const path = item.getField("location").String();
        DcmFileFormat fileformat;
        OFCondition condition = fileformat.loadFile(path.c_str());
        if (condition.bad())
        {
            dopamine::loggerError() << "Unable to load file: " << condition.text();

            createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        DcmDataset* dataset = fileformat.getAndRemoveDataset();
        
        OFString sopclassuid;
        condition = dataset->findAndGetOFString(DCM_SOPClassUID, sopclassuid);
        if (condition.bad())
        {
            dopamine::loggerError() << "Missing SOPClassUID field in dataset: "
                                    << condition.text();

            createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                               DCM_SOPClassUID, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }
        OFString sopinstanceuid;
        condition = dataset->findAndGetOFString(DCM_SOPInstanceUID, sopinstanceuid);
        if (condition.bad())
        {
            dopamine::loggerError() << "Missing DCM_SOPInstanceUID field in dataset: "
                                    << condition.text();

            createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                               DCM_SOPInstanceUID, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }

        // Perform sub operation
        condition = this->performGetSubOperation(sopclassuid.c_str(),
                                                 sopinstanceuid.c_str(),
                                                 dataset);
        if (condition.bad())
        {
            dopamine::loggerError() << "Get Sub-Op Failed: "
                                         << condition.text();

            createStatusDetail(STATUS_GET_Failed_UnableToProcess,
                               DCM_UndefinedTagKey, OFString(condition.text()), details);

            this->_status = STATUS_GET_Failed_UnableToProcess;
            return;
        }

        this->_status = STATUS_Pending;
    }
}

OFCondition 
GetResponseGenerator
::performGetSubOperation(const char* sopClassUID, const char* sopInstanceUID,
                         DcmDataset* dataset)
{
    DIC_US msgID = this->_request_association->nextMsgID++;
    
    /* which presentation context should be used */
    T_ASC_PresentationContextID presID;
    presID = ASC_findAcceptedPresentationContextID(this->_request_association,
                                                   sopClassUID);
    if (presID == 0)
    {
        dopamine::loggerError() << "No presentation context for: "
                                << dcmSOPClassUIDToModality(sopClassUID, "OT");
        return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
    }
    else
    {
        /* make sure that we can send images in this presentation context */
        T_ASC_PresentationContext pc;
        OFCondition condition =
                ASC_findAcceptedPresentationContext(this->_request_association->params,
                                                    presID, &pc);
        if (condition.bad())
        {
            return condition;
        }

        if (pc.acceptedRole != ASC_SC_ROLE_SCP &&
            pc.acceptedRole != ASC_SC_ROLE_SCUSCP)
        {
            dopamine::loggerError() << "No presentation context with requestor SCP role for: "
                                    << dcmSOPClassUIDToModality(sopClassUID, "OT");
            return DIMSE_NOVALIDPRESENTATIONCONTEXTID;
        }
    }
    
    // Create a C-Store request
    T_DIMSE_C_StoreRQ req;
    req.MessageID = msgID;
    strcpy(req.AffectedSOPClassUID, sopClassUID);
    strcpy(req.AffectedSOPInstanceUID, sopInstanceUID);
    req.DataSetType = DIMSE_DATASET_PRESENT;
    req.Priority = this->_priority;
    req.opts = 0;
    
    T_DIMSE_DetectedCancelParameters cancelParameters;
    T_DIMSE_C_StoreRSP rsp;
    DcmDataset* stdetail = NULL;
    
    dopamine::loggerInfo() << "Store SCU RQ: MsgID " << msgID;

    // Send the C-Store request
    return DIMSE_storeUser(this->_request_association, presID, &req, NULL,
                           dataset, getSubProcessCallback, this,
                           DIMSE_BLOCKING, 30, &rsp, &stdetail,
                           &cancelParameters);
}

} // namespace services
    
} // namespace dopamine