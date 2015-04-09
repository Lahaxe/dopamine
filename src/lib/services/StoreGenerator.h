/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _137519da_5031_4188_b52f_b1a3616696c5
#define _137519da_5031_4188_b52f_b1a3616696c5

#include <dcmtk/config/osconfig.h> /* make sure OS specific configuration is included first */
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmnet/dimse.h>

#include "services/Generator.h"

namespace dopamine
{

namespace services
{

class StoreGenerator : public Generator
{
public:
    StoreGenerator(std::string const & username);

    /// Destroy the store response generator
    virtual ~StoreGenerator();

    virtual Uint16 set_query(mongo::BSONObj const & query_dataset);

    void set_callingaptitle(std::string const & callingaptitle);

    mongo::BSONObj dataset_to_bson(DcmDataset * const dataset);

private:
    std::string _destination_path;

    std::string _callingaptitle;

    std::string _sop_instance_uid;

    DcmDataset * _dataset;

    void create_destination_path(DcmDataset *dataset);

    bool is_dataset_allowed_for_storage(mongo::BSONObj const & dataset);

};

} // namespace services

} // namespace dopamine

#endif // _137519da_5031_4188_b52f_b1a3616696c5