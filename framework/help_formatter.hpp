#ifndef HELP_FORMATTER_HDR
#define HELP_FORMATTER_HDR

#include <vector>

#include <iostream>
#include <boost/program_options/options_description.hpp>

#include "main_factories_keeper.hpp"
#include "test_extractor.hpp"

class HelpFormatter {

public:

    void formatHelps(std::ostream& output);
    void formatOneProcessorHelp(std::string processorName, std::ostream& output);
    bool formatProcessorHelpsByName(std::string aliasOrProcessorName, std::ostream& output);

    void formatAliases(std::ostream& output);
    void formatOneAlias(std::string aliasName, std::ostream& output);
    void formatAliasesForProcessor(std::string processorName);

    void formatDescription(std::ostream& output);
    void formatTutorial(std::ostream& output);
    void formatLicence(std::ostream& output);
    void formatAboutPsiFormat(std::ostream& output);
    void formatFAQ(std::ostream& output);
    void formatInstallationGuide(std::ostream& output);

    virtual ~HelpFormatter();

protected:

    TestExtractor testExtractor_;

    virtual void doFormatOneProcessorHelp(
        std::string processorName,
        std::string description,
        std::string detailedDescription,
        boost::program_options::options_description options,
        std::list<std::string> aliases,
        std::vector<TestBatch> usingExamples,
        std::list<std::string> languagesHandled,
        std::ostream& output) =0;

    std::string getProcessorDescription(std::string processorName);
    std::string getProcessorDetailedDescription(std::string processorName);

    boost::program_options::options_description getProcessorOptions(std::string processorName);
    std::vector<TestBatch> getProcessorUsingExamples(std::string processorName);

    virtual void doFormatOneAlias(
        std::string aliasName,
        std::list<std::string> processorNames,
        std::ostream& output) =0;

    std::list<std::string> getProcessorNamesForAlias(std::string alias);
    std::list<std::string> getAliasesForProcessorName(std::string processorName);

    virtual void doFormatDataFile(std::string text, std::ostream& output) =0;

    boost::filesystem::path getPathToFrameworkDataFile_(const std::string& filename);
    std::string getFileContent(const boost::filesystem::path& path);

    const static std::string EXAMPLES_HEADER;
    const static std::string OPTIONS_HEADER;
    const static std::string ALIASES_HEADER;
    const static std::string LANGUAGES_HEADER;

    std::list<std::string> getLanguagesHandledForProcessor(std::string processorName);

};

#endif
