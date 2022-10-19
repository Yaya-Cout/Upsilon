#include "app.hpp"
#include "rahat_icon.h"
#include "apps/apps_container.h"
#include "apps/i18n.h"

namespace rahat
{
    I18n::Message App::Descriptor::name(){
        return I18n::Message::RahatApp;
    }

    I18n::Message App::Descriptor::upperName(){
        return I18n::Message::RahatAppCapital;
    }

    const Image* App::Descriptor::icon(){
        return ImageStore::RahatIcon;
    }

    App* App::Snapshot::unpack(Container* container){
        return new (container->currentAppBuffer()) App(this);
    }

    App::Descriptor* App::Snapshot::descriptor(){
        static App::Descriptor descriptor;
        return &descriptor;
    }

    App::App(Snapshot * snapshot) : ::App(snapshot, &m_CSListCtrl), m_CSListCtrl(nullptr){}
} // namespace rahat
