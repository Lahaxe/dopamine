/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include "core/LoggerPACS.h"
#include "services/FindResponseGenerator.h"
#include "FindSCP.h"

namespace dopamine
{

namespace services
{

/**
 * Callback handler called by the DIMSE_findProvider callback function
 * @param callbackdata: Callback context (in)
 * @param cancelled: flag indicating wheter a C-CANCEL was received (in)
 * @param request: original find request (in)
 * @param requestIdentifiers: original find request identifiers (in)
 * @param responseCount: find response count (in)
 * @param response: final find response (out)
 * @param responseIdentifiers: find response identifiers (out)
 * @param stDetail: status detail for find response (out)
 */
static void findCallback(
        /* in */
        void *callbackData, OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP *response, DcmDataset **responseIdentifiers,
        DcmDataset **stDetail)
{
    FindResponseGenerator* context =
            reinterpret_cast<FindResponseGenerator*>(callbackData);
    context->process(cancelled, request, requestIdentifiers,
                     responseCount, response, responseIdentifiers,
                     stDetail);
}
    
FindSCP
::FindSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID, 
          T_DIMSE_C_FindRQ * req):
    SCP(assoc, presID), _request(req) // base class initialisation
{
    // nothing to do
}

FindSCP
::~FindSCP()
{
    // nothing to do
}

OFCondition 
FindSCP
::process()
{
    dopamine::loggerInfo() << "Received Find SCP: MsgID "
                                << this->_request->MessageID;

    FindResponseGenerator context(this->_association);
    
    return DIMSE_findProvider(this->_association, this->_presentationID,
                              this->_request, findCallback, &context, 
                              DIMSE_BLOCKING, 30);
}

} // namespace services

} // namespace dopamine