/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#define BOOST_TEST_MODULE ModuleAuthenticatorCSV
#include <boost/test/unit_test.hpp>

#include <dcmtkpp/DcmtkAssociation.h>

#include "authenticator/AuthenticatorCSV.h"
#include "core/ExceptionPACS.h"

struct TestDataCSV
{
    std::string filename;

    TestDataCSV():
        filename("./tmp_test_moduleAuthenticatorCSV.csv")
    {
        std::ofstream myfile;
        myfile.open(filename);
        myfile << "user1\tpassword1\n";
        myfile << "user2\tpassword2\n";
        myfile.close();
    }

    ~TestDataCSV()
    {
        remove(filename.c_str());
    }
};

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Constructor / Destructor
 */
BOOST_FIXTURE_TEST_CASE(Constructor, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV* authenticatorcsv =
            new dopamine::authenticator::AuthenticatorCSV(filename);

    BOOST_REQUIRE(authenticatorcsv != NULL);
    
    BOOST_CHECK_EQUAL(authenticatorcsv->get_table_count(), 2);
    
    delete authenticatorcsv;
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Get authorization => true
 */
BOOST_FIXTURE_TEST_CASE(AuthorizationTrue, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV authenticatorcsv(filename);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_to_username_and_password("user2", "password2");
    
    BOOST_CHECK_EQUAL(authenticatorcsv(association), true);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Empty identity => false
 */
BOOST_FIXTURE_TEST_CASE(NoIdentity, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV authenticatorcsv(filename);

    dcmtkpp::DcmtkAssociation association;

    BOOST_CHECK_EQUAL(authenticatorcsv(association), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request with Bad user => false
 */
BOOST_FIXTURE_TEST_CASE(AuthorizationFalse, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV authenticatorcsv(filename);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_to_username_and_password("baduser",
                                                           "password2");

    BOOST_CHECK_EQUAL(authenticatorcsv(association), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request with Bad password => false
 */
BOOST_FIXTURE_TEST_CASE(BadPassword, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV authenticatorcsv(filename);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_to_username_and_password("user2",
                                                           "badpassword");

    BOOST_CHECK_EQUAL(authenticatorcsv(association), false);
}

/******************************* TEST Nominal **********************************/
/**
 * Nominal test case: Request with Bad identity type => false
 */
BOOST_FIXTURE_TEST_CASE(BadIdentityType, TestDataCSV)
{
    dopamine::authenticator::AuthenticatorCSV authenticatorcsv(filename);

    dcmtkpp::DcmtkAssociation association;
    association.set_user_identity_to_kerberos("user2");

    BOOST_CHECK_EQUAL(authenticatorcsv(association), false);
}

/******************************* TEST Error ************************************/
/**
 * Error test case: Construction failure => Unknown file
 */
BOOST_AUTO_TEST_CASE(BadFilename)
{
    BOOST_REQUIRE_THROW(
                dopamine::authenticator::AuthenticatorCSV("badfilename"),
                dopamine::ExceptionPACS);
}
