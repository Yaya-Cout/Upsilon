#ifndef RAHAT_CREATE_CS_CONTROLLER_H
#define RAHAT_CREATE_CS_CONTROLLER_H

#include "escher.h"

namespace Rahat {

class CreateCS_Controller: public ViewController, SimpleListViewDataSource, SelectableTableViewDataSource {

private:
    // View
    SelectableTableView m_tableView;
    // Title Cell
    MessageTableCellWithBuffer m_title;
    // Subject Cell
    MessageTableCellWithBuffer m_subject;
    // Create Button
    MessageTableCellWithChevron<> m_create;

public:
    CreateCS_Controller(Responder * parentResponder);
    View * view() override;

    // Cell Height
    KDCoordinate cellHeight() override;
    // Returns a ptr to a cell
    HighlightCell * reusableCell(int index) override;
    // Max number of cells
    int reusableCellCount() const override;
    // Number of rows displayed on screen
    int numberOfRows() const override;
    // Displays the cell
    void willDisplayCellForIndex(HighlightCell * cell, int index) override;

    // Stuff for events
    void didBecomeFirstResponder() override;
};

}

#endif
