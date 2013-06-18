#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>

#include "apertium_lattice_reader.hpp"
#include "processor_file_fetcher.hpp"


std::string ApertiumLatticeReader::getFormatName() {
    return "Apertium";
}

std::string ApertiumLatticeReader::doInfo() {
    return "Apertium reader";
}

std::string ApertiumLatticeReader::processReplacementRules(const std::string& input) {
    return apertiumDeformatter_.processReplacementRules(input);
}

std::vector<DeformatIndex> ApertiumLatticeReader::processFormatRules(const std::string& input) {
    return apertiumDeformatter_.processFormatRules(input);
}

const std::string ApertiumLatticeReader::Factory::DEFAULT_SPEC_FILES_DIR = "%ITSDATA%/";
const std::string ApertiumLatticeReader::Factory::DEFAULT_SPEC_FILE_ENDING = "-format.xml";

ApertiumLatticeReader::ApertiumLatticeReader(
    const boost::filesystem::path& specificationFile,
    bool unzipData,
    bool keepTags)
    : apertiumDeformatter_(specificationFile, unzipData), keepTags_(keepTags) { }

bool ApertiumLatticeReader::isKeepTags() {
    return keepTags_;
}

ApertiumLatticeReader::Factory::~Factory() { }

LatticeReader<std::istream>* ApertiumLatticeReader::Factory::doCreateLatticeReader(
    const boost::program_options::variables_map& options) {

    std::string specFilePath;

    if (options.count("specification-file")) {
        specFilePath = options["specification-file"].as<std::string>();
    }
    else {
        specFilePath = DEFAULT_SPEC_FILES_DIR + "/"
            + options["format"].as<std::string>() + DEFAULT_SPEC_FILE_ENDING;
        DEBUG("using XML format file: " << specFilePath);
    }

    ProcessorFileFetcher fileFetcher(__FILE__);
    boost::filesystem::path specificationFile = fileFetcher.getOneFile(specFilePath);

    bool unzipData = true;
    if (options.count("unzip-data")) {
        unzipData = options["unzip-data"].as<bool>();
    }

    bool keepTags = false;
    if (options.count("keep-tags")) {
        keepTags = options["keep-tags"].as<bool>();
    }

    return new ApertiumLatticeReader(specificationFile, unzipData, keepTags);
}

boost::program_options::options_description ApertiumLatticeReader::Factory::doOptionsHandled() {
    boost::program_options::options_description optionsDescription(OPTION_LABEL);

    optionsDescription.add_options()
        ("format", boost::program_options::value<std::string>()->default_value("html"),
            "type of file for deformatting")
        ("specification-file", boost::program_options::value<std::string>(),
            "specification file path")
        ("unzip-data", boost::program_options::value<bool>()->default_value(true),
            "unzip compressed file formats like .pptx or .xlsx")
        ("keep-tags", boost::program_options::bool_switch()->default_value(false),
            "keep formatting tags");

    return optionsDescription;
}

std::string ApertiumLatticeReader::Factory::doGetName() const {
    return "apertium-reader";
}

boost::filesystem::path ApertiumLatticeReader::Factory::doGetFile() const {
    return __FILE__;
}

ApertiumLatticeReader::Worker::Worker(ApertiumLatticeReader& processor,
                                      std::istream& inputStream,
                                      Lattice& lattice) :
    ReaderWorker<std::istream>(inputStream, lattice),
    processor_(processor),
    fragTags_(lattice_.getLayerTagManager().createTagCollectionFromList(
        boost::assign::list_of("frag")("apertium-reader"))),
    tagTags_(lattice_.getLayerTagManager().createTagCollectionFromList(
        boost::assign::list_of("tag")("apertium-reader"))) {
}

void ApertiumLatticeReader::Worker::doRun() {
    std::string input;
    std::string line;

    bool firstLine = true;

    while (std::getline(inputStream_, line)) {
        if (boost::algorithm::trim_copy(line).empty()) {
            continue;
        }

        if (firstLine) {
            firstLine = false;
        }
        else {
            input += '\n';
        }

        input += line;
    }

    input = processor_.processReplacementRules(input);
    std::vector<DeformatIndex> indexes = processor_.processFormatRules(input);

    if (!indexes.empty()) {
        std::string text;

        if (indexes[0].begin != 0) {
            text = input.substr(0, indexes[0].begin);
            appendFragmentToLattice_(text);
        }

        for (unsigned int i = 0; i < indexes.size(); i++) {
            text = input.substr(indexes[i].begin, indexes[i].length());

            if (processor_.isKeepTags()) {
                appendTagToLattice_(text, indexes[i].type, indexes[i].eos);
            }

            int length = (i == indexes.size() - 1) ? input.length() : indexes[i+1].begin;
            length -= indexes[i].end;

            if (length != 0) {
                text = input.substr(indexes[i].end, length);
                appendFragmentToLattice_(text);
            }
        }
    }
    else {
        appendFragmentToLattice_(input);
    }
}

void ApertiumLatticeReader::Worker::appendFragmentToLattice_(std::string fragment) {
    if (fragment.empty()) {
        return;
    }

    Lattice::VertexDescriptor prevEnd = lattice_.getLastVertex();
    lattice_.appendStringWithSymbols(fragment);
    Lattice::VertexDescriptor nowEnd = lattice_.getLastVertex();
    AnnotationItem item("FRAG", fragment);
    lattice_.addEdge(prevEnd, nowEnd, item, fragTags_);
}

void ApertiumLatticeReader::Worker::appendTagToLattice_(
    std::string tag, std::string type, bool eos) {

    if (tag.empty()) {
        return;
    }

    Lattice::VertexDescriptor prevEnd = lattice_.getLastVertex();
    lattice_.appendStringWithSymbols(tag);
    Lattice::VertexDescriptor nowEnd = lattice_.getLastVertex();

    AnnotationItem item(boost::to_upper_copy(type) + "-TAG", StringFrag(tag));
    AnnotationItemManager& manager = lattice_.getAnnotationItemManager();
    manager.setValue(item, "eos", eos ? "yes" : "no");

    lattice_.addEdge(prevEnd, nowEnd, item, tagTags_);
}
