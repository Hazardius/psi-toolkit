#include "bracketing_lattice_writer.hpp"


#include <algorithm>
#include <iterator>

#include "bracketing_quoter.hpp"
#include "string_helpers.hpp"


std::string BracketingLatticeWriter::getFormatName() {
    return "bracketing";
}


LatticeWriter<std::ostream>* BracketingLatticeWriter::Factory::doCreateLatticeWriter(
    const boost::program_options::variables_map& options
) {
    BracketingQuoter quoter;

    std::vector<std::string> showOnlyTags;
    std::vector<std::string> filter;
    std::vector<std::string> showAttributes;

    if (options.count("show-only-tags")) {
        showOnlyTags = options["show-only-tags"].as< std::vector<std::string> >();
    } else if (options.count("filter")) {
        showOnlyTags = options["filter"].as< std::vector<std::string> >();
    }

    if (options.count("filter")) {
        filter = options["filter"].as< std::vector<std::string> >();
    }
    if (options.count("show-attributes")) {
        showAttributes = options["show-attributes"].as< std::vector<std::string> >();
    }

    return new BracketingLatticeWriter(
        quoter.unescape(options["opening-bracket"].as<std::string>()),
        quoter.unescape(options["closing-bracket"].as<std::string>()),
        quoter.unescape(options["tag-separator"].as<std::string>()),
        showOnlyTags,
        filter,
        quoter.unescape(options["av-pairs-separator"].as<std::string>()),
        quoter.unescape(options["av-separator"].as<std::string>()),
        showAttributes,
        !options.count("skip-symbol-edges"),
        !options.count("with-blank")
    );
}


boost::program_options::options_description BracketingLatticeWriter::Factory::doOptionsHandled() {
    boost::program_options::options_description optionsDescription(OPTION_LABEL);

    optionsDescription.add_options()
        ("opening-bracket",
            boost::program_options::value<std::string>()->default_value("["),
            "the actual format of opening brackets")
        ("closing-bracket",
            boost::program_options::value<std::string>()->default_value("]"),
            "the actual format of closing brackets")
        ("tag-separator",
            boost::program_options::value<std::string>()->default_value(","),
            "separates tags")
        ("show-only-tags",
            boost::program_options::value< std::vector<std::string> >()->multitoken(),
            "limits the tag names that will appear in `%T` substitions")
        ("filter",
            boost::program_options::value< std::vector<std::string> >()->multitoken(),
            "filters the edges by tags")
        ("av-pairs-separator",
            boost::program_options::value<std::string>()->default_value(","),
            "separates the attribute-value pairs")
        ("av-separator",
            boost::program_options::value<std::string>()->default_value("="),
            "separates the attribute and its value")
        ("show-attributes",
            boost::program_options::value< std::vector<std::string> >()->multitoken(),
            "the attributes to be shown")
        ("skip-symbol-edges",
            "skip symbol edges")
        ("with-blank",
            "do not skip edges with whitespace text");

    return optionsDescription;
}


std::string BracketingLatticeWriter::Factory::doGetName() const {
    return "bracketing-writer";
}


boost::filesystem::path BracketingLatticeWriter::Factory::doGetFile() const {
    return __FILE__;
}


BracketingLatticeWriter::BracketingLatticeWriter(
    std::string openingBracket,
    std::string closingBracket,
    std::string tagSeparator,
    std::vector<std::string> showOnlyTags,
    std::vector<std::string> filter,
    std::string avPairsSeparator,
    std::string avSeparator,
    std::vector<std::string> showAttributes,
    bool showSymbolEdges,
    bool skipBlank
) :
    openingBracket_(openingBracket),
    closingBracket_(closingBracket),
    tagSeparator_(tagSeparator),
    showOnlyTags_(showOnlyTags.begin(), showOnlyTags.end()),
    filter_(filter.begin(), filter.end()),
    avPairsSeparator_(avPairsSeparator),
    avSeparator_(avSeparator),
    showAttributes_(showAttributes.begin(), showAttributes.end()),
    showSymbolEdges_(showSymbolEdges),
    skipBlank_(skipBlank)
{ }


bool BracketingLatticeWriter::areSomeInFilter(std::list<std::string> & tags) {
    if (filter_.empty()) return true;
    BOOST_FOREACH(std::string tag, tags) {
        if (filter_.count(tag)) return true;
    }
    return false;
}


std::set<std::string> BracketingLatticeWriter::intersectOnlyTags(
    std::set<std::string> tags
) {
    if (showOnlyTags_.empty()) {
        return tags;
    }
    std::set<std::string> result;
    set_intersection(
        showOnlyTags_.begin(), showOnlyTags_.end(),
        tags.begin(), tags.end(),
        std::inserter(result, result.begin())
    );
    return result;
}


std::map<std::string, std::string> BracketingLatticeWriter::filterAttributes(
    std::map<std::string, std::string> avMap
) {
    if (showAttributes_.empty()) {
        return avMap;
    }
    std::map<std::string, std::string> result;
    typedef std::pair<std::string, std::string> StrStrPair;
    BOOST_FOREACH(StrStrPair avPair, avMap) {
        if (showAttributes_.count(avPair.first)) {
            result.insert(avPair);
        }
    }
    return result;
}


std::string BracketingLatticeWriter::doInfo() {
    return "Bracketing writer";
}


BracketingLatticeWriter::Worker::Worker(BracketingLatticeWriter& processor,
                                 std::ostream& outputStream,
                                 Lattice& lattice):
    AligningWriterWorker(outputStream, lattice), processor_(processor) {
}


void BracketingLatticeWriter::Worker::doRun() {

    std::vector<std::string> patterns;
    patterns.push_back(processor_.getOpeningBracket());
    patterns.push_back(processor_.getClosingBracket());

    BracketPrinter bracketPrinter(
        patterns,
        processor_.getTagSeparator(),
        processor_.getAVPairsSeparator(),
        processor_.getAVSeparator()
    );

    std::string latticeText = lattice_.getAllText();
    size_t latticeSize = latticeText.length() + 1;

    std::set<EdgeData> * * edgeStore = new std::set<EdgeData> * [latticeSize];
    for (size_t i = 0; i < latticeSize; i++) {
        edgeStore[i] = new std::set<EdgeData>[latticeSize];
    }

    std::string * * printedBrackets = new std::string * [latticeSize];
    for (size_t i = 0; i < latticeSize; i++) {
        printedBrackets[i] = new std::string [latticeSize];
    }

    Lattice::EdgesSortedBySourceIterator ei
        = lattice_.edgesSortedBySource(lattice_.getLayerTagManager().anyTag());
    while (ei.hasNext()) {
        Lattice::EdgeDescriptor edge = ei.next();
        std::list<std::string> tagNames
            = lattice_.getLayerTagManager().getTagNames(lattice_.getEdgeLayerTags(edge));
        if (
            tagNames.size() == 1 &&
            tagNames.front() == "symbol" &&
            !processor_.isShowSymbolEdges()
        ) continue;
        if (
            processor_.isSkipBlank() &&
            lattice_.isBlank(edge)
        ) continue;
        if (!processor_.areSomeInFilter(tagNames)) continue;
        int begin = lattice_.getEdgeBeginIndex(edge);
        int end = lattice_.getEdgeEndIndex(edge);
        edgeStore[begin][end].insert(getEdgeData_(edge));
    }

    for (size_t i = 0; i < latticeSize; i += symbolLength(latticeText, i)) {
        for (size_t j = 0; j < latticeSize; j += symbolLength(latticeText, j)) {
            if (i < j) {
                std::set< std::vector<std::string> > printed
                    = bracketPrinter.print(edgeStore[i][j]);
                BOOST_FOREACH(std::vector<std::string> p, printed) {
                    printedBrackets[i][j] = printedBrackets[i][j] + p[0];
                    printedBrackets[j][i] = p[1] + printedBrackets[j][i];
                }
            }
        }
    }

    for (size_t i = 0; i < latticeSize; i += symbolLength(latticeText, i)) {
        for (
            size_t j = ((i + latticeSize) - 1) % latticeSize;
            j != i;
            j = ((j + latticeSize) - 1) % latticeSize
        ) {
            print_(printedBrackets[i][j]);
        }
        print_(latticeText.substr(i, symbolLength(latticeText, i)));
    }

    for (size_t i = 0; i < latticeSize; i++) {
        delete [] printedBrackets[i];
    }
    delete [] printedBrackets;

    for (size_t i = 0; i < latticeSize; i++) {
        delete [] edgeStore[i];
    }
    delete [] edgeStore;

}


BracketingLatticeWriter::Worker::~Worker() {
}


EdgeData BracketingLatticeWriter::Worker::getEdgeData_(Lattice::EdgeDescriptor edge) {
    AnnotationItem annotationItem = lattice_.getEdgeAnnotationItem(edge);
    std::set<std::string> tags;
    std::list<std::string> tagsList
        = lattice_.getLayerTagManager().getTagNames(lattice_.getEdgeLayerTags(edge));
    tags.insert(tagsList.begin(), tagsList.end());
    std::map<std::string, std::string> avMap;
    avMap = lattice_.getAnnotationItemManager().getAVMap(annotationItem);
    return EdgeData(
        lattice_,
        edge,
        processor_.intersectOnlyTags(tags),
        annotationItem.getCategory(),
        annotationItem.getText(),
        processor_.filterAttributes(avMap),
        lattice_.getEdgeScore(edge)
    );
}
