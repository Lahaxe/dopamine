/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _ee9915a2_a504_4b21_8d43_7938c66c526e
#define _ee9915a2_a504_4b21_8d43_7938c66c526e

#include "services/ResponseGenerator.h"

namespace dopamine
{

namespace services
{

/**
 * @brief Response Generator for C-FIND services.
 */
class FindResponseGenerator : public ResponseGenerator
{
public :
    /// Create a default find response generator
    FindResponseGenerator(T_ASC_Association * request_association);
    
    /// Destroy the find response generator
    virtual ~FindResponseGenerator();
    
    /**
     * Callback handler called by the DIMSE_findProvider callback function
     * @param cancelled: flag indicating whether a C-CANCEL was received (in)
     * @param request: original find request (in)
     * @param requestIdentifiers: original find request identifiers (in)
     * @param responseCount: find response count (in)
     * @param response: final find response (out)
     * @param responseIdentifiers: find response identifiers (out)
     * @param stDetail: status detail for find response (out)
     */
    void process(
        /* in */
        OFBool cancelled, T_DIMSE_C_FindRQ* request,
        DcmDataset* requestIdentifiers, int responseCount,
        /* out */
        T_DIMSE_C_FindRSP* response,
        DcmDataset** responseIdentifiers, DcmDataset** stDetail);

protected:
    /**
     * Process next response
     * @param responseIdentifiers: find response identifiers (out)
     */
    virtual void next(DcmDataset ** responseIdentifiers, DcmDataset ** details);

private :
    
};

} // namespace services

} // namespace dopamine

#endif // _ee9915a2_a504_4b21_8d43_7938c66c526e