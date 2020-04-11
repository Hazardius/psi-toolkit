#include "niema.hpp"

#include <climits>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign/list_of.hpp>

#include "config.hpp"
#include "lang_specific_processor_file_fetcher.hpp"
#include "psi_exception.hpp"


const std::string Niema::Factory::AUXILLIARY_GRM_PATH
    = "%ITSDATA%/%LANG%/aux.grm";

const std::string Niema::Factory::DEFAULT_FAR_PATH
    = "%ITSDATA%/%LANG%/all.far";

const std::string Niema::Factory::DEFAULT_CONDITIONS_PATH
    = "%ITSDATA%/%LANG%/conditions.txt";

const std::string Niema::Factory::DEFAULT_EXCEPTIONS_PATH
    = "%ITSDATA%/%LANG%/exceptions.lst";

const std::string Niema::Factory::FAR_2018_PATH
    = "%ITSDATA%/%LANG%/rules_2018.far";

const std::string Niema::Factory::CONDITIONS_2018_PATH
    = "%ITSDATA%/%LANG%/conditions_2018.txt";

const std::string Niema::Factory::FAR_2020_PATH
    = "%ITSDATA%/%LANG%/rules_2020.far";

const std::string Niema::Factory::CONDITIONS_2020_PATH
    = "%ITSDATA%/%LANG%/conditions_2020.txt";

const std::list<std::string> Niema::requiredTags = boost::assign::list_of
        ("iayko");

const std::list<std::string> Niema::tagsToOperateOn = boost::assign::list_of
        ("iayko");

const std::list<std::string> Niema::providedTags = boost::assign::list_of
        ("niema")("normalization");

const std::list<std::string> Niema::tagsToPut = boost::assign::list_of
        ("niema")("normalization")("token")("conditional");


std::string Niema::Factory::getRealFileName(std::string fileSpec, std::string lang) const
{
    LangSpecificProcessorFileFetcher fileFetcher(__FILE__, lang);
    return fileFetcher.getOneFile(fileSpec).string();
}


Annotator* Niema::Factory::doCreateAnnotator(
    const boost::program_options::variables_map& options)
{
    std::string lang = options["lang"].as<std::string>();
    std::string farFileSpec = options["far"].as<std::string>();
    std::string fst = options["fst"].as<std::string>();
    std::string condition = options["condition"].as<std::string>();
    std::string conditionsFileSpec = options["conditions"].as<std::string>();
    std::string grm = options["grm"].as<std::string>();
    std::string md = options["md"].as<std::string>();
    std::string saveFar = options["save-far"].as<std::string>();
    std::string saveConditions = options["save-conditions"].as<std::string>();
    std::string exceptionsFileSpec = options["exceptions"].as<std::string>();
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

    if (!fst.empty() && conditionsFileSpec != DEFAULT_CONDITIONS_PATH) {
        throw PsiException("Options --fst and --conditions must not be used together");
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
        if (conditionsFileSpec == DEFAULT_CONDITIONS_PATH) {
            conditionsFileSpec = CONDITIONS_2018_PATH;
        }
    } else if (version == "2020") {
        if (farFileSpec == DEFAULT_FAR_PATH) {
            farFileSpec = FAR_2020_PATH;
        }
        if (conditionsFileSpec == DEFAULT_CONDITIONS_PATH) {
            conditionsFileSpec = CONDITIONS_2020_PATH;
        }
    }

    if (!grm.empty() && !md.empty()) {
        throw PsiException("Options --grm and --md must not be used together");
    }

    Niema::Spec spec;
    if (options.count("spec")) {
        std::vector<std::string> optspec = options["spec"].as< std::vector<std::string> >();
        std::vector<std::string>::iterator si = optspec.begin();

        while (si != optspec.end()) {
            std::string far = getRealFileName(*si, lang);

            ++si;
            if (si == optspec.end()) {
                break;
            }
            std::string fst = *si;

            ++si;
            if (si == optspec.end()) {
                break;
            }
            std::string condition = *si;

            spec.push_back(std::make_pair(std::make_pair(far, fst), condition));
            ++si;
        }
    }

    if (!spec.empty()) {
        if (!fst.empty()) {
            throw PsiException("Options --fst and --spec must not be used together");
        }

        if (conditionsFileSpec != DEFAULT_CONDITIONS_PATH) {
            throw PsiException("Options --conditions and --spec must not be used together");
        }

        return new Niema(lang, spec, exceptions);
    }

    std::string far = getRealFileName(farFileSpec, lang);
    std::string conditions = getRealFileName(conditionsFileSpec, lang);

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
        std::string command;
        std::string comment;
        std::string condition;
        while (std::getline(fin, line)) {
            if (boost::starts_with(line, "    ")) {
                command = line.substr(4);
            } else if (boost::starts_with(line, "\t")) {
                command = line.substr(1);
            } else if (boost::starts_with(line, "%")) {
                condition = line.substr(1);
            } else if (line.empty()) {
                fout << std::endl;
            } else {
                comment = line;
            }

            if (!command.empty()) {
                if (!condition.empty() && boost::starts_with(command, "export ")) {
                    size_t pos = command.find_first_of(" =", 7);
                    std::string fst = command.substr(7, pos - 7);
                    spec.push_back(std::make_pair(std::make_pair(saveFar, fst), condition));
                }
                fout << command;
            }
            if (!comment.empty()) {
                fout << "# " << comment;
            }
            fout << std::endl;

            command = "";
            comment = "";
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
        std::ifstream fin(conditions.c_str());
        if (!fin.is_open()) {
            throw FileFormatException(std::string("Cannot open file ") + conditions);
        }
        std::string conditionLine;
        while (std::getline(fin, conditionLine)) {
            if (!conditionLine.empty()) {
                std::vector<std::string> conditionLinePair;
                boost::split(
                        conditionLinePair,
                        conditionLine,
                        boost::is_any_of("\t"));
                spec.push_back(std::make_pair(std::make_pair(far, conditionLinePair[0]), conditionLinePair[1]));
            }
        }
        fin.close();

        if (!saveConditions.empty()) {
            std::ofstream fout(saveConditions.c_str());
            for (Niema::Spec::iterator si = spec.begin();
                    si != spec.end();
                    ++si) {
                fout << si->first.second << "\t" << si->second << std::endl;
            }
            fout.close();
        }

        return new Niema(lang, spec, exceptions);
    }

    if (spec.empty()) {
        spec.push_back(std::make_pair(std::make_pair(far, fst), condition));
    }
    return new Niema(lang, spec, exceptions);
}


void Niema::Factory::doAddLanguageIndependentOptionsHandled(
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
    ("condition",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "condition for fst")
    ("conditions",
        boost::program_options::value<std::string>()
        ->default_value(DEFAULT_CONDITIONS_PATH),
        "file with conditions")
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
    ("save-conditions",
        boost::program_options::value<std::string>()
        ->default_value(std::string()),
        "where to save the conditions to file")
    ("bypass-exceptions",
        "bypass exceptions")
    ("exceptions",
        boost::program_options::value<std::string>()
        ->default_value(DEFAULT_EXCEPTIONS_PATH),
        "a text file with list of exceptions")
    ("version", boost::program_options::value<std::string>()
        ->default_value("2017"),
        "rules version (2017/2018/2020)")
    ;
}


std::string Niema::Factory::doGetName() const
{
    return "niema";
}


std::string Niema::Factory::doGetSubType() const
{
    return "normalizer";
}


std::list<std::string> Niema::Factory::doGetAliases()
{
    return boost::assign::list_of<std::string>
        (std::string("niema-normalizer"))
        (std::string("fst-normalizer"))
        (std::string("thrax-normalizer"));
}


boost::filesystem::path Niema::Factory::doGetFile() const
{
    return __FILE__;
}


std::list<std::list<std::string> > Niema::Factory::doRequiredLayerTags()
{
    return boost::assign::list_of(Niema::requiredTags);
}


std::list<std::list<std::string> > Niema::Factory::doOptionalLayerTags()
{
    return std::list<std::list<std::string> >();
}


std::list<std::string> Niema::Factory::doProvidedLayerTags()
{
    return Niema::providedTags;
}


std::list<std::string> Niema::Factory::doAllLanguagesHandled() const {
    return boost::assign::list_of
        (std::string("pl"));
}


std::string Niema::Factory::doGetContinuation(
    const boost::program_options::variables_map& /* options */) const
{
    return "simple-writer --tags niema --sep \" \"";
}


bool Niema::Factory::doCheckRequirements(
    const boost::program_options::variables_map& options,
    std::ostream& message) const
{
    return PluginManager::getInstance().checkPluginRequirements("openfst",
            options,
            message);
}


LatticeWorker* Niema::doCreateLatticeWorker(Lattice& lattice)
{
    return new Worker(*this, lattice);
}


Niema::Worker::Worker(Processor& processor, Lattice& lattice):
    LatticeWorker(lattice), processor_(processor),
    outputTags_(lattice_.getLayerTagManager().createTagCollectionFromListWithLangCode(
                  Niema::tagsToPut, dynamic_cast<Niema&>(processor_).langCode_))
{
}


void Niema::Worker::doRun()
{
    Niema& niemaProcessor = dynamic_cast<Niema&>(processor_);

    if (niemaProcessor.isActive())
    {
        LayerTagManager& ltm = lattice_.getLayerTagManager();
        AnnotationItemManager& aim = lattice_.getAnnotationItemManager();

        LayerTagMask tokenMask_ = ltm.getMaskWithLangCode(
                Niema::tagsToOperateOn, niemaProcessor.langCode_);

        Lattice::EdgesSortedByTargetIterator edgeIter
            = lattice_.edgesSortedByTarget(tokenMask_);

        Lattice::EdgeDescriptor lastTokenEdge;
        Lattice::EdgeDescriptor lastSeparatingEdge;

        std::list<Lattice::EdgeSpec> edgesToAdd;
        std::vector<Lattice::VertexDescriptor> verticesToUse;

        while (edgeIter.hasNext())
        {
            Lattice::EdgeDescriptor currentEdge = edgeIter.next();
            std::string category = lattice_.getAnnotationCategory(currentEdge);

            Lattice::VertexDescriptor source = lattice_.getEdgeSource(currentEdge);
            int sourceSpec = verticesToUse.size();
            verticesToUse.push_back(source);

            Lattice::VertexDescriptor target = lattice_.getEdgeTarget(currentEdge);
            int targetSpec = verticesToUse.size();
            verticesToUse.push_back(target);

            std::string text = lattice_.getAnnotationText(currentEdge);

            for (Spec::iterator si = niemaProcessor.spec_.begin();
                    si != niemaProcessor.spec_.end();
                    ++si) {
                std::string normalized_text;
                if (text == " "
                        || std::find(niemaProcessor.exceptions_.begin(),
                            niemaProcessor.exceptions_.end(),
                            text) != niemaProcessor.exceptions_.end()) {
                    normalized_text = text;
                } else {
                    normalized_text = fstNormalize_(
                            si->first.first,
                            si->first.second,
                            text);
                }
                std::string condition = si->second;

                if (normalized_text != text) {
                    std::vector<std::string> normalized_text_tokenized;
                    boost::split(
                            normalized_text_tokenized,
                            normalized_text,
                            boost::is_any_of(" "));
                    size_t normalized_parts = normalized_text_tokenized.size();

                    std::vector<std::string> conditions;
                    boost::split(
                            conditions,
                            condition,
                            boost::is_any_of("+"));
                    size_t condition_parts = conditions.size();

                    if (!condition.empty() && normalized_parts != condition_parts) {
                        std::stringstream errorSs;
                        // errorSs << "Condition \"" << condition << "\" cannot be"
                        //     << " applied for text \"" << normalized_text << "\"";
                        // throw FileFormatException(errorSs.str());
                    } else {

                        if (normalized_parts == 1) {
                            AnnotationItem ai("T", normalized_text);
                            if (!condition.empty()) {
                                aim.setValue(ai, "condition", condition);
                            }
                            edgesToAdd.push_back(Lattice::EdgeSpec(
                                    sourceSpec,
                                    targetSpec,
                                    ai,
                                    outputTags_));
                        } else {
                            int currentVertexSpec;
                            int nextVertexSpec;
                            for (size_t i = 0; i < normalized_parts; ++i) {
                                if (i > 0) {
                                    AnnotationItem aiBlank("B", std::string(" "));
                                    currentVertexSpec = nextVertexSpec;
                                    nextVertexSpec = verticesToUse.size();
                                    verticesToUse.push_back(INT_MAX);
                                    edgesToAdd.push_back(Lattice::EdgeSpec(
                                            currentVertexSpec,
                                            nextVertexSpec,
                                            aiBlank,
                                            outputTags_));
                                }

                                AnnotationItem ai("T", normalized_text_tokenized[i]);
                                if (!condition.empty()) {
                                    aim.setValue(ai, "condition", conditions[i]);
                                }
                                if (i == 0) {
                                    currentVertexSpec = sourceSpec;
                                } else {
                                    currentVertexSpec = nextVertexSpec;
                                }
                                if (i == normalized_parts - 1) {
                                    nextVertexSpec = targetSpec;
                                } else {
                                    nextVertexSpec = verticesToUse.size();
                                    verticesToUse.push_back(INT_MAX);
                                }
                                edgesToAdd.push_back(Lattice::EdgeSpec(
                                        currentVertexSpec,
                                        nextVertexSpec,
                                        ai,
                                        outputTags_));
                            }
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < verticesToUse.size(); ++i) {
            if (verticesToUse[i] == INT_MAX) {
                verticesToUse[i] = lattice_.addLooseVertex();
            }
        }

        for (std::list<Lattice::EdgeSpec>::iterator esIter = edgesToAdd.begin();
                esIter != edgesToAdd.end();
                ++esIter) {
            lattice_.addEdge(
                    verticesToUse[esIter->fromSpec],
                    verticesToUse[esIter->toSpec],
                    esIter->annotationItem,
                    esIter->tags);
        }
    }
}


std::string Niema::Worker::fstNormalize_(
        const std::string& far,
        const std::string& fst,
        const std::string& text)
{
    Niema& niemaProcessor = dynamic_cast<Niema&>(processor_);
    return niemaProcessor.getAdapter()->normalize(far, fst, text);
}


std::string Niema::doInfo()
{
    return "Niema";
}


Niema::Niema(const std::string& langCode,
             Niema::Spec spec,
             std::vector<std::string> exceptions)
{
    init_(langCode, spec, exceptions);

    if ( isActive() ) {
        openFSTAdapter_->init();
    }
}


Niema::~Niema()
{
    if (openFSTAdapter_)
    {
        PluginManager::getInstance().destroyPluginAdapter("openfst", openFSTAdapter_);
    }
}


void Niema::init_(const std::string& langCode,
                  Niema::Spec spec,
                  std::vector<std::string> exceptions)
{
    langCode_ = langCode;
    spec_ = spec;
    exceptions_ = exceptions;

    openFSTAdapter_ = dynamic_cast<OpenFSTAdapterInterface*>(
            PluginManager::getInstance().createPluginAdapter("openfst"));
}


OpenFSTAdapterInterface* Niema::getAdapter()
{
    return openFSTAdapter_;
}


bool Niema::isActive()
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
