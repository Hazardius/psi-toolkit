#include "iayko.hpp"

#include <climits>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>

#include "config.hpp"
#include "lang_specific_processor_file_fetcher.hpp"
#include "psi_exception.hpp"


const std::string Iayko::Factory::AUXILLIARY_GRM_PATH
    = "%ITSDATA%/%LANG%/aux.grm";

const std::string Iayko::Factory::DEFAULT_FAR_PATH
    = "%ITSDATA%/%LANG%/all.far";

const std::string Iayko::Factory::DEFAULT_FSTS_PATH
    = "%ITSDATA%/%LANG%/rules.txt";

const std::string Iayko::Factory::FAR_2018_PATH
    = "%ITSDATA%/%LANG%/rules2018.far";

const std::string Iayko::Factory::FSTS_2018_PATH
    = "%ITSDATA%/%LANG%/rules2018.txt";

const std::string Iayko::Factory::FAR_2020_PATH
    = "%ITSDATA%/%LANG%/rules2020.far";

const std::string Iayko::Factory::FSTS_2020_PATH
    = "%ITSDATA%/%LANG%/rules2020.txt";

const std::string Iayko::Factory::DEFAULT_EXCEPTIONS_PATH
    = "%ITSDATA%/%LANG%/exceptions.lst";

const std::string Iayko::Factory::DEFAULT_IN_TAG
    = "token";

const std::list<std::string> Iayko::requiredTags = boost::assign::list_of
        ("token");

const std::list<std::string> Iayko::providedTags = boost::assign::list_of
        ("iayko")("normalization");

const std::list<std::string> Iayko::tagsToPut = boost::assign::list_of
        ("iayko")("normalization")("token");


std::string Iayko::Factory::getRealFileName(std::string fileSpec, std::string lang) const
{
    LangSpecificProcessorFileFetcher fileFetcher(__FILE__, lang);
    return fileFetcher.getOneFile(fileSpec).string();
}


Annotator* Iayko::Factory::doCreateAnnotator(
    const boost::program_options::variables_map& options)
{
    std::string lang = options["lang"].as<std::string>();
    std::string farFileSpec = options["far"].as<std::string>();
    std::string fst = options["fst"].as<std::string>();
    std::string fstsFileSpec = options["fsts"].as<std::string>();
    std::string grm = options["grm"].as<std::string>();
    std::string md = options["md"].as<std::string>();
    std::string saveFar= options["save-far"].as<std::string>();
    std::string exceptionsFileSpec = options["exceptions"].as<std::string>();
    std::string inTag = options["in-tag"].as<std::string>();
    std::string version = options["version"].as<std::string>();

    std::vector<std::string> exceptions;
    if (options.count("bypass-exceptions")) {
        std::string exceptionsFile = getRealFileName(exceptionsFileSpec, lang);

        std::ifstream fin(exceptionsFile.c_str());
        if (!fin.is_open()) {
            throw FileFormatException(
                    std::string("Cannot open file ") + exceptionsFile);
        }
        std::string exception;
        while (std::getline(fin, exception)) {
            if (!exception.empty()) {
                exceptions.push_back(exception);
            }
        }
        fin.close();
    }

    if (!fst.empty() && fstsFileSpec != DEFAULT_FSTS_PATH) {
        throw PsiException("Options --fst and --fsts must not be used together");
    }

    if (farFileSpec != DEFAULT_FAR_PATH && !grm.empty()) {
        throw PsiException("Options --far and --grm must not be used together");
    }

    if (farFileSpec != DEFAULT_FAR_PATH && !md.empty()) {
        throw PsiException("Options --far and --md must not be used together");
    }

    if (version == "2018") {
        if (farFileSpec == DEFAULT_FAR_PATH) {
            farFileSpec = FAR_2018_PATH;
        }
        if (fstsFileSpec == DEFAULT_FSTS_PATH) {
            fstsFileSpec = FSTS_2018_PATH;
        }
    } else if (version == "2020") {
        if (farFileSpec == DEFAULT_FAR_PATH) {
            farFileSpec = FAR_2020_PATH;
        }
        if (fstsFileSpec == DEFAULT_FSTS_PATH) {
            fstsFileSpec = FSTS_2020_PATH;
        }
    }

    if (!grm.empty() && !md.empty()) {
        throw PsiException("Options --grm and --md must not be used together");
    }

    std::vector< std::pair<std::string, std::string> > transducersSpec;
    if (options.count("spec")) {
        std::vector<std::string> spec = options["spec"].as< std::vector<std::string> >();
        std::vector<std::string>::iterator si = spec.begin();
        while (si != spec.end()) {
            std::string far = getRealFileName(*si, lang);
            ++si;
            if (si == spec.end()) {
                break;
            }
            std::string fst = *si;
            transducersSpec.push_back(std::make_pair(far, fst));
            ++si;
        }
    }

    if (!transducersSpec.empty()) {
        if (!fst.empty()) {
            throw PsiException("Options --fst and --spec must not be used together");
        }

        if (fstsFileSpec != DEFAULT_FSTS_PATH) {
            throw PsiException("Options --fsts and --spec must not be used together");
        }

        return new Iayko(lang, transducersSpec, exceptions, inTag);
    }

    std::string far = getRealFileName(farFileSpec, lang);
    std::string fsts = getRealFileName(fstsFileSpec, lang);

    if ((!grm.empty() || !md.empty()) && saveFar.empty()) {
        static char tmpFileNameTemplate[] = "far_XXXXXX";
        char tmpFileName[PATH_MAX];
        strcpy(tmpFileName, tmpFileNameTemplate);
        mkstemp(tmpFileName);
        saveFar = tmpFileName;
    }

    if (!md.empty()) {
        static char tmpFileNameTemplate[] = "grm_XXXXXX";
        char tmpFileName[PATH_MAX];
        strcpy(tmpFileName, tmpFileNameTemplate);
        mkstemp(tmpFileName);
        grm = tmpFileName;

        std::ifstream fin(md.c_str());
        if (!fin.is_open()) {
            throw FileFormatException(std::string("Cannot open file ") + md);
        }

        std::ofstream fout(grm.c_str());
        if (!fout.is_open()) {
            throw FileFormatException(std::string("Cannot open file ") + grm);
        }

        std::ifstream auxin(getRealFileName(AUXILLIARY_GRM_PATH, lang).c_str());
        if (auxin.is_open()) {
            fout << auxin.rdbuf();
            auxin.close();
        } else {
            WARN("Cannot open file " + AUXILLIARY_GRM_PATH);
        }

        std::string line;
        while (std::getline(fin, line)) {
            if (boost::starts_with(line, "    ")) {
                fout << line.substr(4) << std::endl;
            } else if (boost::starts_with(line, "\t")) {
                fout << line.substr(1) << std::endl;
            } else if (line.empty()) {
                fout << line << std::endl;
            } else {
                fout << "# " << line << std::endl;
            }
        }
        fin.close();
        fout.close();
    }

    if (!grm.empty()) {
        std::stringstream commandSs;
        commandSs << "thraxcompiler --input_grammar=" << grm
            << " --output_far=" << saveFar << " 1>&2";
        system(commandSs.str().c_str());

        far = saveFar;
    }

    if (fst.empty()) {
        std::ifstream fin(fsts.c_str());
        if (!fin.is_open()) {
            throw FileFormatException(std::string("Cannot open file ") + fsts);
        }
        std::string fstLine;
        while (std::getline(fin, fstLine)) {
            if (!fstLine.empty()) {
                transducersSpec.push_back(std::make_pair(far, fstLine));
            }
        }
        fin.close();

        return new Iayko(lang, transducersSpec, exceptions, inTag);
    }

    return new Iayko(lang, far, fst, exceptions, inTag);
}


void Iayko::Factory::doAddLanguageIndependentOptionsHandled(
    boost::program_options::options_description& optionsDescription)
{
    optionsDescription.add_options()
    ("far",
        boost::program_options::value<std::string>()
        ->default_value(DEFAULT_FAR_PATH),
        "far archive with rules")
    ("fst",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "fst name inside far")
    ("fsts",
        boost::program_options::value<std::string>()
        ->default_value(DEFAULT_FSTS_PATH),
        "file with fst names to be used as a cascade")
    ("spec",
        boost::program_options::value< std::vector<std::string> >()->multitoken(),
        "specification of more far:fst pairs to be used as cascade")
    ("grm",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "text file with rules written in Thrax")
    ("md",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "text file with rules written in Thrax and their description in Markdown")
    ("save-far",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "where to save the far archive compiled from grm file")
    ("bypass-exceptions",
        "bypass exceptions")
    ("exceptions",
        boost::program_options::value<std::string>()
        ->default_value(DEFAULT_EXCEPTIONS_PATH),
        "a text file with list of exceptions")
    ("in-tag", boost::program_options::value<std::string>()
        ->default_value(DEFAULT_IN_TAG),
        "tag to operate on")
    ("version", boost::program_options::value<std::string>()
        ->default_value("2017"),
        "rules version (2017/2018/2020)")
    ;
}


std::string Iayko::Factory::doGetName() const
{
    return "iayko";
}


std::string Iayko::Factory::doGetSubType() const
{
    return "normalizer";
}


std::list<std::string> Iayko::Factory::doGetAliases()
{
    return boost::assign::list_of<std::string>
        (std::string("iayko-normalizer"))
        (std::string("fst-normalizer"))
        (std::string("thrax-normalizer"));
}


boost::filesystem::path Iayko::Factory::doGetFile() const
{
    return __FILE__;
}


std::list<std::list<std::string> > Iayko::Factory::doRequiredLayerTags()
{
    return boost::assign::list_of(Iayko::requiredTags);
}


std::list<std::list<std::string> > Iayko::Factory::doOptionalLayerTags()
{
    return std::list<std::list<std::string> >();
}


std::list<std::string> Iayko::Factory::doProvidedLayerTags()
{
    return Iayko::providedTags;
}


std::list<std::string> Iayko::Factory::doAllLanguagesHandled() const {
    return boost::assign::list_of
        (std::string("pl"));
}


std::string Iayko::Factory::doGetContinuation(
    const boost::program_options::variables_map& /* options */) const
{
    return "simple-writer --tags iayko --sep \" \"";
}


bool Iayko::Factory::doCheckRequirements(
    const boost::program_options::variables_map& options,
    std::ostream& message) const
{
    return PluginManager::getInstance().checkPluginRequirements("openfst",
            options,
            message);
}


LatticeWorker* Iayko::doCreateLatticeWorker(Lattice& lattice)
{
    return new Worker(*this, lattice);
}


Iayko::Worker::Worker(Processor& processor, Lattice& lattice):
    LatticeWorker(lattice), processor_(processor),
    outputTags_(lattice_.getLayerTagManager().createTagCollectionFromListWithLangCode(
                  Iayko::tagsToPut, dynamic_cast<Iayko&>(processor_).langCode_))
{
}


void Iayko::Worker::doRun()
{
    Iayko& iaykoProcessor = dynamic_cast<Iayko&>(processor_);

    if (iaykoProcessor.isActive())
    {
        LayerTagManager& ltm = lattice_.getLayerTagManager();

        LayerTagMask tokenMask = ltm.getMaskWithLangCode(
                iaykoProcessor.inTag_, iaykoProcessor.langCode_);

        Lattice::EdgesSortedByTargetIterator edgeIter
            = lattice_.edgesSortedByTarget(tokenMask);

        Lattice::EdgeDescriptor lastTokenEdge;
        Lattice::EdgeDescriptor lastSeparatingEdge;

        while (edgeIter.hasNext())
        {
            Lattice::EdgeDescriptor currentEdge = edgeIter.next();
            std::string category = lattice_.getAnnotationCategory(currentEdge);

            Lattice::VertexDescriptor source = lattice_.getEdgeSource(currentEdge);
            Lattice::VertexDescriptor target = lattice_.getEdgeTarget(currentEdge);

            std::string text = lattice_.getAnnotationText(currentEdge);

            std::string normalized_text;
            if (std::find(iaykoProcessor.exceptions_.begin(),
                        iaykoProcessor.exceptions_.end(),
                        text) != iaykoProcessor.exceptions_.end()) {
                normalized_text = text;
            } else {
                normalized_text = fstNormalize_(text);
            }

            AnnotationItem ai("T", normalized_text);
            lattice_.addEdge(
                    source,
                    target,
                    ai,
                    outputTags_);

        }
    }
}


std::string Iayko::Worker::fstNormalize_(const std::string& text)
{
    Iayko& iaykoProcessor = dynamic_cast<Iayko&>(processor_);
    return iaykoProcessor.getAdapter()->normalize(text);
}


std::string Iayko::doInfo()
{
    return "Iayko";
}


Iayko::Iayko(const std::string& langCode,
             const std::string& far,
             const std::string& fst,
             std::vector<std::string> exceptions,
             const std::string& inTag)
    : inTag_(inTag)
{
    init_(langCode, exceptions);

    if ( isActive() ) {
        openFSTAdapter_->init(far, fst);
    }
}


Iayko::Iayko(const std::string& langCode,
             std::vector< std::pair<std::string, std::string> > spec,
             std::vector<std::string> exceptions,
             const std::string& inTag)
    : inTag_(inTag)
{
    init_(langCode, exceptions);

    if ( isActive() ) {
        openFSTAdapter_->init(spec);
    }
}


Iayko::~Iayko()
{
    if (openFSTAdapter_)
    {
        PluginManager::getInstance().destroyPluginAdapter("openfst", openFSTAdapter_);
    }
}


void Iayko::init_(const std::string& langCode,
                  std::vector<std::string> exceptions)
{
    langCode_ = langCode;
    exceptions_ = exceptions;

    openFSTAdapter_ = dynamic_cast<OpenFSTAdapterInterface*>(
            PluginManager::getInstance().createPluginAdapter("openfst"));
}


OpenFSTAdapterInterface* Iayko::getAdapter()
{
    return openFSTAdapter_;
}


bool Iayko::isActive()
{
    if (openFSTAdapter_)
    {
        return true;
    }
    else
    {
        return false;
    }
}
