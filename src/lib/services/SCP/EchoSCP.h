/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _fc8d9861_729f_4ef3_9723_9588628a4ee4
#define _fc8d9861_729f_4ef3_9723_9588628a4ee4

#include <dcmtkpp/SCP.h>

namespace dopamine
{

namespace services
{

class EchoSCP : public dcmtkpp::SCP
{
public:
    EchoSCP();

    virtual void operator()(dcmtkpp::message::Message const & message);
};

} // namespace services

} // namespace dopamine

#endif // _fc8d9861_729f_4ef3_9723_9588628a4ee4
