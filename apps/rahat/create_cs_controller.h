#ifndef CREATE_CS_CONTROLLER_H
# define CREATE_CS_CONTROLLER_H

#include "escher.h"

namespace rahat {

    class CreateCS_Controller : public ViewController, SimpleListViewDataSource, SelectableTableViewDataSource {

        private:
            // Vue
            SelectableTableView m_tableView;
            // Cellule "Titre"
            MessageTableCellWithBuffer m_title;
            // Cellule "Matière"
            MessageTableCellWithBuffer m_subject;
            // Bouton "Créer"
            MessageTableCellWithChevron<> m_create;
        
        public:
            CreateCS_Controller(Responder* parentResponder);
            View* view() override;

            // Largeur de la cellule
            KDCoordinate cellHeight() override;
            // Renvoie un ptr vers une cellule
            HighlightCell* reusableCell(int index) override;
            // Nbr max de cellules
            int reusableCellCount() const override;
            // Nbr de rangées affichées à l'écran
            int numberOfRows() const override;
            // Affiche la cellule
            void willDisplayCellForIndex(HighlightCell * cell, int index) override;

            // Truc pour les events
            void didBecomeFirstResponder() override;
    };
    
    
} // namespace rahat


#endif