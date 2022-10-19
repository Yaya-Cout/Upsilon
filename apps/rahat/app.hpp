#include "escher.h"
#include "cs_list_controller.hpp"

namespace rahat {

    class App : public ::App {
        public:

            // Sert à décrire l'appli (nom, icône)
            class Descriptor : public ::App::Descriptor {
                public:
                    I18n::Message name() override;

                    I18n::Message upperName() override;

                    const Image* icon() override;
            };

            // Enregistre l'appli
            class Snapshot : public ::App::Snapshot {
                public:
                    App* unpack(Container* container) override;

                    Descriptor * descriptor() override;
            };

        private:
            App(Snapshot* snapshot);
            CSListController m_CSListCtrl;
    };

}