#include "link_parser.hpp"

#include <boost/assign.hpp>

#include "lang_specific_processor_file_fetcher.hpp"
#include "plugin_manager.hpp"


Annotator* LinkParser::Factory::doCreateAnnotator(
    const boost::program_options::variables_map& options
) {
    std::string lang = options["lang"].as<std::string>();
    LangSpecificProcessorFileFetcher fileFetcher(__FILE__, lang);

    std::string dictPathString;
    if (options.count("dict")) {
        std::string dictFilename = options["dict"].as<std::string>();
        boost::filesystem::path dictPath = fileFetcher.getOneFile(dictFilename);
        dictPathString = dictPath.string();
    }

    return new LinkParser(dictPathString);
}

void LinkParser::Factory::doAddLanguageIndependentOptionsHandled(
    boost::program_options::options_description& optionsDescription) {
    optionsDescription.add_options()
        ("dict",
        boost::program_options::value<std::string>()->default_value(DEFAULT_DICT_FILE),
        "dictionary file")
        ;
}

std::string LinkParser::Factory::doGetName() const {
    return "link-parser";
}

boost::filesystem::path LinkParser::Factory::doGetFile() const {
    return __FILE__;
}

std::list<std::list<std::string> > LinkParser::Factory::doRequiredLayerTags() {
    return std::list<std::list<std::string> >();
}

std::list<std::list<std::string> > LinkParser::Factory::doOptionalLayerTags() {
    return std::list<std::list<std::string> >();
}

std::list<std::string> LinkParser::Factory::doProvidedLayerTags() {
    return boost::assign::list_of("link-grammar")("parse");
}

const std::string LinkParser::Factory::DEFAULT_DICT_FILE
    = "%ITSDATA%/%LANG%.dict";

LatticeWorker* LinkParser::doCreateLatticeWorker(Lattice& lattice) {
    return new Worker(*this, lattice);
}

LinkParser::Worker::Worker(LinkParser& processor, Lattice& lattice) :
    LatticeWorker(lattice), processor_(processor)
{
    //TODO
}

void LinkParser::Worker::doRun() {
    processor_.parse(lattice_);
}

std::string LinkParser::doInfo() {
    return "link grammar parser";
}

LinkParser::LinkParser(std::string dictPath) {
    adapter_ = dynamic_cast<LinkParserAdapterInterface*>(
        PluginManager::getInstance().createPluginAdapter("link-parser")
    );
    adapter_->setDictionary(dictPath);
}

LinkParser::~LinkParser() {
    if (adapter_) {
        PluginManager::getInstance().destroyPluginAdapter("link-parser", adapter_);
    }
}

LinkParserAdapterInterface * LinkParser::getAdapter() {
    return adapter_;
}

bool LinkParser::isActive() {
    if (adapter_) {
        return true;
    } else {
        return false;
    }
}


void LinkParser::parse(Lattice &lattice) {
    LayerTagMask maskSegment = lattice.getLayerTagManager().getMask("segment");
    Lattice::EdgesSortedBySourceIterator ei(lattice, maskSegment);
    while (ei.hasNext()) {
        Lattice::EdgeDescriptor edge = ei.next();
        std::string parsed(adapter_->parseSentence(lattice.getEdgeText(edge)));
        AnnotationItem aiLink(parsed);
        LayerTagCollection tagParse = lattice.getLayerTagManager().createTagCollectionFromList(
            boost::assign::list_of("link-grammar")("parse")
        );
        lattice.addEdge(
            lattice.getEdgeSource(edge),
            lattice.getEdgeTarget(edge),
            aiLink,
            tagParse
        );
    }
}