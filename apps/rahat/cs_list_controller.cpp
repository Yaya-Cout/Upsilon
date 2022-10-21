#include "cs_list_controller.h"
#include "apps/i18n.h"

namespace Rahat {
CSListController::CSListController(Responder * parentResponder) :
    ViewController(parentResponder),
    m_tableView(this, this, this),
    m_createCSCtrl(this) {
    sheets[0].name = "Foo";
    sheets[1].name = "Bar";
    sheets[2].name = "Baz";
    sheetNb = 3;
}

View * CSListController::view() {
    return &m_tableView;
}

int CSListController::numberOfRows() const {
    return sheetNb + NB_SETTINGS - 1;
}

KDCoordinate CSListController::cellHeight() {
    return 50;
}

HighlightCell * CSListController::reusableCell(int index) {
    if (index == 0) return &m_setting_cell[0];

    return &m_csList[index];
}

int CSListController::reusableCellCount() const {
    return NB_CELLS;
}

void CSListController::willDisplayCellForIndex(HighlightCell * cell, int index) {

    if (index == 0) {
        MessageTableCellWithChevron<> * mySetting = static_cast<MessageTableCellWithChevron<>*>(cell);
        MessageTextView * myTextView = static_cast<MessageTextView *>(mySetting->labelView());
        myTextView->setMessage(I18n::Message::NewCS);
    } else {
        MessageTableCell<> * myTextCell = static_cast<MessageTableCell<>*>(cell);
        MessageTextView * myTextView = static_cast<MessageTextView *>(myTextCell->labelView());
        myTextView->setText(sheets[index - 1].name);
        myTextCell->setMessageFont(KDFont::LargeFont);
    }
}

void CSListController::didBecomeFirstResponder() {
    if (selectedRow() < 0) {
        selectCellAtLocation(0, 0);
    }

    Container::activeApp()->setFirstResponder(&m_tableView);
}

bool CSListController::handleEvent(Ion::Events::Event event) {
    if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::Right) {
        static_cast<StackViewController *>(parentResponder())->push(&m_createCSCtrl);
        Container::activeApp()->setFirstResponder(&m_createCSCtrl);
        return true;
    }

    return false;
}

}
