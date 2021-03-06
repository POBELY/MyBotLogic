#ifndef SELECTEUR_H
#define SELECTEUR_H

#include "../BT_Composite.h"

class Selecteur : public BT_Composite {

public:
   Selecteur() = default;
   Selecteur(vector<BT_Noeud*> noeuds) : BT_Composite(noeuds) {
   }

   BT_Noeud::ETAT_ELEMENT execute() noexcept override {
	   int ind = 0;
	   BT_Noeud::ETAT_ELEMENT res;
	   do {
		   res = noeuds[ind]->execute();
	   } while (++ind < noeuds.size() && res == BT_Noeud::ETAT_ELEMENT::ECHEC);

      return res;
   }
};

#endif