#ifndef RAHAT_CS_LIST_CONTROLLER_H
#define RAHAT_CS_LIST_CONTROLLER_H

#include "escher.h"
#include "apps/external/archive.h"
#include "sheet.h"
#include "setting.h"
#include "create_cs_controller.h"

namespace Rahat {

class CSListController: public ViewController, public SimpleListViewDataSource, public SelectableTableViewDataSource {
private:
    // View : TableView
    SelectableTableView m_tableView;

    // CS's
    static const int NB_CS = 20;
    Rahat::Sheet sheets[NB_CS];
    int sheetNb = 0;

    // CS Cells
    static const int NB_CELLS = 6;
    MessageTableCell<> m_csList[NB_CELLS];

    // Settings cells
    static const int NB_SETTINGS = 2;
    Rahat::Setting m_settings[NB_SETTINGS];
    MessageTableCellWithChevron<> m_setting_cell[NB_SETTINGS];

    // Create a CS
    CreateCS_Controller m_createCSCtrl;

public:
    CSListController(Responder * parentResponder);

    View * view() override;

    KDCoordinate cellHeight() override;
    HighlightCell * reusableCell(int index) override;
    int reusableCellCount() const override;
    int numberOfRows() const override;
    void willDisplayCellForIndex(HighlightCell * cell, int index) override;

    void didBecomeFirstResponder() override;

    bool handleEvent(Ion::Events::Event event) override;
};

}

#endif
