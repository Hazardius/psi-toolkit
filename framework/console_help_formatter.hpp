#ifndef CONSOLE_HELP_FORMATTER_HDR
#define CONSOLE_HELP_FORMATTER_HDR

#include "help_formatter.hpp"

class ConsoleHelpFormatter : public HelpFormatter {

public:
    ~ConsoleHelpFormatter();

protected:
    void doFormatOneProcessorHelp(
        std::string processorName,
        std::string description,
        std::string detailedDescription,
        boost::program_options::options_description options,
        std::list<std::string> aliases,
        std::vector<TestBatch> usingExamples,
        std::list<std::string> languagesHandled,
        std::ostream& output);

    void doFormatOneAlias(
        std::string aliasName,
        std::list<std::string> processorNames,
        std::ostream& output);

    void doFormatDataFile(std::string text, std::ostream& output);

private:

    void formatAliases_(std::list<std::string> aliases, std::ostream& output);
    void formatLanguagesHandled_(std::list<std::string> langCodes, std::ostream& output);

};

#endif
