#include "escher.h"
#include "apps/external/archive.h"
#include "sheet.hpp"
#include "setting.hpp"
#include "create_cs_controller.h"

namespace rahat
{
    class CSListController : public ViewController, public SimpleListViewDataSource, public SelectableTableViewDataSource {
        private:
            // View : TableView
            SelectableTableView m_tableView;

            // CS's
            static const int NB_CS = 20;
            rahat::Sheet sheets[NB_CS];
            int sheetNb = 0;

            // CS Cells
            static const int NB_CELLS = 6;
            MessageTableCell<> m_csList[NB_CELLS];

            // Settings cells
            static const int NB_SETTINGS = 2;
            rahat::Setting m_settings[NB_SETTINGS];
            MessageTableCellWithChevron<> m_setting_cell[NB_SETTINGS];

            // Create a CS
            CreateCS_Controller m_createCSCtrl;
            
        public:
            CSListController(Responder* parentResponder);
            
            View* view() override;

            KDCoordinate cellHeight() override;
            HighlightCell* reusableCell(int index) override;
            int reusableCellCount() const override;
            int numberOfRows() const override;
            void willDisplayCellForIndex(HighlightCell * cell, int index) override;

            void didBecomeFirstResponder() override;

            bool handleEvent(Ion::Events::Event event) override;
    };
} // namespace rahat
