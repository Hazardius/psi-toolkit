#ifndef IAYKO_HDR
#define IAYKO_HDR

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "annotator.hpp"
#include "language_dependent_annotator_factory.hpp"
#include "annotator_factory.hpp"

#include "plugin/openfst_adapter_interface.hpp"
#include "plugin_manager.hpp"

class Iayko : public Annotator
{

public:
    class Factory : public LanguageDependentAnnotatorFactory
    {
    private:
        virtual Annotator* doCreateAnnotator(
            const boost::program_options::variables_map& options);

        virtual void doAddLanguageIndependentOptionsHandled(
            boost::program_options::options_description& optionsDescription);

        virtual std::string doGetName() const;
        virtual std::string doGetSubType() const;
        virtual std::list<std::string> doGetAliases();
        virtual boost::filesystem::path doGetFile() const;

        virtual std::list<std::list<std::string> > doRequiredLayerTags();

        virtual std::list<std::list<std::string> > doOptionalLayerTags();

        virtual std::list<std::string> doProvidedLayerTags();

        virtual std::string doGetContinuation(
            const boost::program_options::variables_map& options) const;

        virtual std::list<std::string> doAllLanguagesHandled() const;

        virtual bool doCheckRequirements(
            const boost::program_options::variables_map& options,
            std::ostream & message) const;

        std::string getRealFileName(std::string fileSpec, std::string lang) const;

        static const std::string AUXILLIARY_GRM_PATH;
        static const std::string DEFAULT_FAR_PATH;
        static const std::string DEFAULT_FSTS_PATH;
        static const std::string FAR_2018_PATH;
        static const std::string FSTS_2018_PATH;
        static const std::string FAR_2020_PATH;
        static const std::string FSTS_2020_PATH;
        static const std::string DEFAULT_EXCEPTIONS_PATH;
        static const std::string DEFAULT_IN_TAG;
    };

    Iayko(const std::string& langCode,
          const std::string& far,
          const std::string& fst,
          std::vector< std::string > exceptions,
          const std::string& inTag);
    Iayko(const std::string& langCode,
          std::vector< std::pair<std::string, std::string> > spec,
          std::vector< std::string > exceptions,
          const std::string& inTag);
    ~Iayko();

    OpenFSTAdapterInterface* getAdapter();
    bool isActive();

    static const std::list<std::string> requiredTags;
    static const std::list<std::string> providedTags;
    static const std::list<std::string> tagsToPut;
private:

    class Worker : public LatticeWorker
    {
    public:
        Worker(Processor& processor, Lattice& lattice);
    private:
        virtual void doRun();
        Processor& processor_;
        LayerTagCollection outputTags_;

        std::string fstNormalize_(const std::string& text);
    };

    virtual LatticeWorker* doCreateLatticeWorker(Lattice& lattice);

    virtual std::string doInfo();

    void init_(const std::string& langCode,
               std::vector< std::string > exceptions);

    std::string langCode_;
    std::vector< std::string > exceptions_;
    std::string inTag_;
    OpenFSTAdapterInterface* openFSTAdapter_;
};


#endif
