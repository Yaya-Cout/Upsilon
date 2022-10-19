#include "cs_list_controller.hpp"
#include "apps/i18n.h"
#include <string>

namespace rahat
{
    CSListController::CSListController(Responder* parentResponder) : ViewController(parentResponder), m_tableView(this, this, this){
        sheets[0].name = "Coucou!!";
        sheets[1].name = "Slay.";
        sheetNb = 2;
    }

    View* CSListController::view(){
        return &m_tableView;
    }

    int CSListController::numberOfRows() const {
        return sheetNb+NB_SETTINGS;
    }

    KDCoordinate CSListController::cellHeight() {
        return 50;
    }

    HighlightCell* CSListController::reusableCell(int index) {
        if (index == 0) return &m_setting_cell[0];
        
        return &m_csList[index];
    }

    int CSListController::reusableCellCount() const {
        return NB_CELLS;
    }

    void CSListController::willDisplayCellForIndex(HighlightCell * cell, int index) {
        
        if (index == 0) {
            MessageTableCellWithChevron<>* mySetting = static_cast<MessageTableCellWithChevron<>*>(cell);
            MessageTextView* myTextView = static_cast<MessageTextView*>(mySetting->labelView());
            myTextView->setMessage(I18n::Message::NewCS);
        } else {
            MessageTableCell<>* myTextCell = static_cast<MessageTableCell<>*>(cell);
            MessageTextView* myTextView = static_cast<MessageTextView*>(myTextCell->labelView());
            myTextView->setText(sheets[index-1].name);
            myTextCell->setMessageFont(KDFont::LargeFont);
        }
    }

    void CSListController::didBecomeFirstResponder(){
        if (selectedRow() < 0){
            selectCellAtLocation(0,0);
        }

        Container::activeApp()->setFirstResponder(&m_tableView);
    }
} // namespace rahat
