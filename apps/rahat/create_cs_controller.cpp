#include "create_cs_controller.h"
#include "apps/i18n.h"

namespace rahat
{
    CreateCS_Controller::CreateCS_Controller(Responder* parentResponder) :
    ViewController(parentResponder),
    m_tableView(this, this, this) {
        // Il n'y a rien pour l'instant... 
    }

    View* CreateCS_Controller::view(){
        return &m_tableView;
    }

    KDCoordinate CreateCS_Controller::cellHeight() {
        return 50;
    }

    HighlightCell* CreateCS_Controller::reusableCell (int index) {
        if (index == 0)
        {
            return &m_title;
        }
        else if (index == 1)
        {
            return &m_subject;
        }
        else
        {
            return &m_create;
        }
        return nullptr;
    }

    int CreateCS_Controller::reusableCellCount() const {
        return 3;
    }

    int CreateCS_Controller::numberOfRows() const {
        return 3;
    }

    void CreateCS_Controller::willDisplayCellForIndex(HighlightCell* cell, int index) {
        if (index == 0)
        {
            MessageTableCellWithBuffer* myCell = static_cast<MessageTableCellWithBuffer*>(cell);
            MessageTextView* myView = static_cast<MessageTextView*>(myCell->labelView());
            myView->setMessage(I18n::Message::Title);
        }
        else if (index == 1)
        {
            MessageTableCellWithBuffer* myCell = static_cast<MessageTableCellWithBuffer*>(cell);
            MessageTextView* myView = static_cast<MessageTextView*>(myCell->labelView());
            myView->setMessage(I18n::Message::Subject);
        }
        else
        {
            MessageTableCellWithChevron<>* myCell = static_cast<MessageTableCellWithChevron<>*>(cell);
            MessageTextView* myView = static_cast<MessageTextView*>(myCell->labelView());
            myView->setMessage(I18n::Message::Create);
        }
        
    }

    void CreateCS_Controller::didBecomeFirstResponder() {
        if (selectedRow() < 0){
            selectCellAtLocation(0,0);
        }

        Container::activeApp()->setFirstResponder(&m_tableView);
    }

} // namespace rahat
