/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _36bbbced_f7c3_45e7_aa64_89253348c949
#define _36bbbced_f7c3_45e7_aa64_89253348c949

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <iconv.h>

namespace research_pacs
{

/**
 * @brief Base class to Convert Bson to Dataset or Dataset to Bson
 */
class ConverterBSONDataSet
{
public:
    /// Create an instance of ConverterBSONDataSet
    ConverterBSONDataSet(bool isDcmToBSON);
    
    /// Destroy the instance of ConverterBSONDataSet
    virtual ~ConverterBSONDataSet();

    /**
     * Get Specific character set
     * @return Specific character set
     */
    std::string get_specific_character_set() const;
    
    /**
     * Set Specific character set
     * @param specific_character_set: new character set
     */
    virtual void set_specific_character_set(std::string const & specific_character_set);

protected:
    /**
     * Get the converter
     * @return converter
     */
    iconv_t get_converter() const { return this->_converter; }
    
    /**
     * Set the converter
     * @param converter: new converter
     */
    void set_converter(iconv_t const & converter) { this->_converter = converter; }

private:
    /// Flag sens of convertion
    bool _isDcmToBSON;

    /// Converter
    iconv_t _converter;
    
    /// Character Set
    std::string _specific_character_set;
    
    static const std::map<std::string, std::string> _dicom_to_iconv;

    /// @brief Generate a map from DICOM encoding to IConv encoding
    static std::map<std::string, std::string> _create_encoding_map();

};

} // namespace research_pacs

#endif // _36bbbced_f7c3_45e7_aa64_89253348c949