/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <fstream>

#include <dcmtk/config/osconfig.h>    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmqrdb/dcmqropt.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmnet/diutil.h>

#include "FindResponseGenerator.h"
#include "FindSCP.h"

namespace research_pacs
{
    
static void findCallback(
        /* in */
        void *callbackData,
        OFBool cancelled, T_DIMSE_C_FindRQ *request,
        DcmDataset *requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP *response,
        DcmDataset **responseIdentifiers,
        DcmDataset **stDetail)
{
    FindResponseGenerator* context = reinterpret_cast<FindResponseGenerator*>(callbackData);
    context->callBackHandler(cancelled, request, requestIdentifiers, 
                             responseCount, response, responseIdentifiers, 
                             stDetail);
}
    
FindSCP
::FindSCP(T_ASC_Association * assoc, 
          T_ASC_PresentationContextID presID, 
          T_DIMSE_C_FindRQ * req):
    SCP(assoc, presID), _request(req)
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
    std::cout << "Received Find SCP: MsgID " 
              << this->_request->MessageID << std::endl;

    DIC_AE aeTitle;
    aeTitle[0] = '\0';
    ASC_getAPTitles(this->_association->params, NULL, aeTitle, NULL);

    FindResponseGenerator context(this, std::string(aeTitle));
    
    return DIMSE_findProvider(this->_association, this->_presentationID, 
                              this->_request, findCallback, &context, 
                              DIMSE_BLOCKING, 0);
}

} // namespace research_pacs