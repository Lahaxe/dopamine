/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#include <dcmtkpp/message/CGetRequest.h>
#include <dcmtkpp/message/CGetResponse.h>

#include "core/LoggerPACS.h"
#include "GetGenerator.h"
#include "ServicesTools.h"

namespace dopamine
{

namespace services
{

GetGenerator::Pointer
GetGenerator
::New()
{
    return Pointer(new GetGenerator());
}

GetGenerator
::GetGenerator():
    GeneratorPACS(), _query_retrieve_level(""), _instance_count_tags({}), _include_fields({}), _maximum_results(0), _skipped_results(0)
{
    // Nothing else.
}

GetGenerator
::~GetGenerator()
{
    // Nothing to do.
}

dcmtkpp::Value::Integer
GetGenerator
::initialize(dcmtkpp::Association const & association,
             dcmtkpp::message::Message const & message)
{
    dcmtkpp::Value::Integer status = GeneratorPACS::initialize(association, message);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    dcmtkpp::message::CGetRequest getrequest(message);
    mongo::BSONObj query_object = dataset_to_bson(getrequest.get_data_set());

    return this->initialize(query_object);
}

dcmtkpp::Value::Integer
GetGenerator
::next()
{
    mongo::BSONObj current_bson = this->_cursor->next();

    if (current_bson.isValid() && current_bson.isEmpty())
    {
        // We're done.
        return dcmtkpp::message::CGetResponse::Success;
    }
    else if (current_bson.hasField("$err"))
    {
        dopamine::logger_warning()
                << "An error occured while processing Get operation: "
                << current_bson.getField("$err").String();
        return dcmtkpp::message::CGetResponse::ProcessingFailure;
    }
    else
    {
        auto data_set = this->_retrieve_dataset(current_bson);
        this->_meta_information = data_set.first;
        this->_current_dataset = data_set.second;
    }

    return dcmtkpp::message::CGetResponse::Pending;
}

dcmtkpp::Value::Integer
GetGenerator
::initialize(mongo::BSONObj const & request)
{
    dcmtkpp::Value::Integer status = GeneratorPACS::initialize(request);
    if (status != dcmtkpp::message::Response::Success)
    {
        return status;
    }

    if (!is_authorized(this->_connection, this->_db_name,
                       this->_username,
                       dcmtkpp::message::Message::Command::C_GET_RQ))
    {
        logger_warning() << "User '" << this->_username
                         << "' not allowed to perform Get Operation";
        return dcmtkpp::message::CGetResponse::RefusedNotAuthorized;
    }

    mongo::BSONObj const constraint = get_constraint_for_user(
                this->_connection, this->_db_name, this->_username,
                dcmtkpp::message::Message::Command::C_GET_RQ);

    mongo::BSONObj query_object = request;

    // Always include the keys for the query level and its higher levels
    if (!query_object.hasField("00080052"))
    {
        logger_warning() << "Cannot find field QueryRetrieveLevel";
        return dcmtkpp::message::CGetResponse::MissingAttribute;
    }
    // Read the Query Retrieve Level
    {
    mongo::BSONObj const field_00080052 =
            query_object.getField("00080052").Obj();
    this->_query_retrieve_level =
            field_00080052.getField("Value").Array()[0].String();
    }

    // Remove unused elements
    query_object =
            query_object.removeField("00080005"); // SpecificCharacterSet
    query_object =
            query_object.removeField("00080052"); // QueryRetrieveLevel

    // Build the MongoDB query and query fields from the query dataset.
    mongo::BSONObjBuilder db_query;
    mongo::BSONObjBuilder fields_builder;

    mongo::BSONArrayBuilder arraybuilder;
    for(mongo::BSONObj::iterator it = query_object.begin(); it.more();)
    {
        mongo::BSONObjBuilder db_query_sub;

        mongo::BSONElement const element = it.next();
        mongo::BSONObj const bsonobj = element.Obj();

        // Always include the field in the results
        fields_builder << element.fieldName() << 1;
        std::string const vr = bsonobj.getField("vr").String();

        std::string fieldtoget = "Value";
        if (vr == "OB" || vr == "OF" || vr == "OW" || vr == "UN")
        {
            fieldtoget = "InlineBinary";
        }

        mongo::BSONElement const & value = bsonobj.getField(fieldtoget);
        Match::Type const match_type = this->_get_match_type(vr, value);

        DicomQueryToMongoQuery function =
                this->_get_query_conversion(match_type);

        std::stringstream field;
        field << std::string(element.fieldName()) << "." << fieldtoget;
        if (vr == "PN")
        {
            field << ".Alphabetic";
        }
        // Match the array element containing the value
        (this->*function)(field.str(), vr, value, db_query_sub);

        arraybuilder << db_query_sub.obj();
    }
    db_query << "$and" << arraybuilder.arr();

    // retrieve 'Content' field
    fields_builder << "Content" << 1;

    // Always include Specific Character Set in results.
    if(!fields_builder.hasField("00080005"))
    {
        fields_builder << "00080005" << 1;
    }

    if(!fields_builder.hasField("00100020"))
    {
        fields_builder << "00100020" << 1;
    }
    if((this->_query_retrieve_level=="STUDY" ||
        this->_query_retrieve_level=="SERIES" ||
        this->_query_retrieve_level=="IMAGE") &&
       !fields_builder.hasField("0020000d"))
    {
        fields_builder << "0020000d" << 1;
    }
    if ((this->_query_retrieve_level=="SERIES" ||
         this->_query_retrieve_level=="IMAGE") &&
        !fields_builder.hasField("0020000e"))
    {
        fields_builder << "0020000e" << 1;
    }
    if (!fields_builder.hasField("00080018"))
    {
        fields_builder << "00080018" << 1;
    }

    for (std::string const field : this->_include_fields)
    {
        if (!fields_builder.hasField(field))
        {
            fields_builder << field << 1;
        }
    }

    // Number of XXX Related Instances (0020,120X)
    std::vector<std::string> const tags = {"00201200", "00201202", "00201204",
                                           "00201206", "00201208", "00201209"};
    for (std::string const tag : tags)
    {
        if (query_object.hasField(tag))
        {
            this->_instance_count_tags.push_back(tag);
        }
    }

    // Create Query
    mongo::BSONArrayBuilder finalquerybuilder;
    finalquerybuilder << constraint << db_query.obj();
    mongo::Query const query(BSON("$and" << finalquerybuilder.arr()));

    // Get fields to retrieve
    mongo::BSONObj const fields = fields_builder.obj();

    // Searching into database...
    this->_cursor = this->_connection.query(this->_db_name + ".datasets",
                                            query, this->_maximum_results,
                                            this->_skipped_results, &fields);

    return dcmtkpp::message::CGetResponse::Pending;
}

void GetGenerator::set_query_retrieve_level(std::string const & query_retrieve_level)
{
    this->_query_retrieve_level = query_retrieve_level;
}

std::string GetGenerator::get_query_retrieve_level() const
{
    return this->_query_retrieve_level;
}

std::vector<std::string> GetGenerator::get_instance_count_tags() const
{
    return this->_instance_count_tags;
}

void GetGenerator::set_include_fields(const std::vector<std::string> &include_fields)
{
    this->_include_fields = include_fields;
}

std::vector<std::string> &GetGenerator::get_include_fields()
{
    return this->_include_fields;
}

void GetGenerator::set_maximum_results(int maximum_results)
{
    this->_maximum_results = maximum_results;
}

int GetGenerator::get_maximum_results() const
{
    return this->_maximum_results;
}

void GetGenerator::set_skipped_results(int skipped_results)
{
    this->_skipped_results = skipped_results;
}

int GetGenerator::get_skipped_results() const
{
    return this->_skipped_results;
}

std::pair<dcmtkpp::DataSet, dcmtkpp::DataSet> GetGenerator::_retrieve_dataset(mongo::BSONObj const & object)
{
    mongo::BSONObj localobject = object;
    if (this->_query_retrieve_level != "IMAGE" &&
        std::find(this->_include_fields.begin(),
                  this->_include_fields.end(),
                  "00080018") != this->_include_fields.end())
    {
        localobject.removeField("00080018");
    }
    return bson_to_dataset(this->_connection, this->_db_name, localobject);
}

} // namespace services

} // namespace dopamine