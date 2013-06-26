#ifndef APERTIUM_FORMAT_SPECIFICATION_HDR
#define APERTIUM_FORMAT_SPECIFICATION_HDR

#include <string>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "format_rules.hpp"
#include "xml_property_tree.hpp"
#include "psi_exception.hpp"

class FormatSpecification {
public:

    FormatSpecification(std::string name, FormatOptions options,
        std::pair<std::vector<FormatRule>, std::vector<ReplacementRule> > rules);

    FormatOptions getOptions();
    std::vector<int> getLevels();
    FormatRule getFormatRule(int i);
    int formatRuleSize();

    std::vector<std::string> formatRulesRegexp();
    std::map<std::string, std::string> replacementRulesRegexp();

    static int const MAX_RULES_PER_LEVEL;

private:

    std::string name_;
    FormatOptions formatOptions_;
    std::vector<FormatRule> formatRules_;
    std::vector<ReplacementRule> replacementRules_;

    std::string getFormatRulesRegexpStartedFrom_(unsigned int, bool);
    bool isTheLastRule_(unsigned int);

    std::vector<int> levels_;
    void setLevels_();
};


class FormatSpecificationReader {
public:

    class Exception : public PsiException {
    public:
        Exception(const std::string& msg) : PsiException(msg) { }
        virtual ~Exception() throw() { }
    };

    class UnexpectedElementException : public Exception {
    public:
        UnexpectedElementException(const std::string& elementName)
            : Exception(std::string("unexpected element `") + elementName + "'") { }
    };

    FormatSpecificationReader(const boost::filesystem::path& filePath);
    FormatSpecification readFormatSpecification();

private:

    boost::shared_ptr<XmlPropertyTree> xmlParsed_;

    std::string parseName_();
    FormatOptions parseOptions_();
    std::pair<std::vector<FormatRule>, std::vector<ReplacementRule> > parseRules_();

    FormatRule parseFormatRule_(boost::property_tree::ptree&);
    ReplacementRule parseReplacementRule_(boost::property_tree::ptree&);

    bool yesNoToBool_(std::string &);
    void removeQuotations_(std::string&);
    void removeBackreferences_(std::string&);
};

#endif
