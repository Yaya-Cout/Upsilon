#ifndef RAHAT_APP_H
#define RAHAT_APP_H

#include "escher.h"
#include "cs_list_controller.h"

namespace Rahat {

class App: public ::App {
public:
    // Describe the app (name, icon)
    class Descriptor: public ::App::Descriptor {
    public:
        I18n::Message name() override;

        I18n::Message upperName() override;

        const Image * icon() override;
    };

    // Register app
    class Snapshot: public ::App::Snapshot {
    public:
        App * unpack(Container * container) override;

        Descriptor * descriptor() override;
    };
private:
    App(Snapshot * snapshot);

    CSListController m_CSListCtrl;
    StackViewController m_stackViewCtrl;
};

}

#endif
