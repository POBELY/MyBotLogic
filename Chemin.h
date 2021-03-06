#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
using namespace std;

class chemin_vide {}; // Exception lev� lorsqu'une op�ration ill�gale est appel�e sur un chemin vide

// Un chemin est constitu� d'un vecteur contenant les indices des cases adjacentes sur lesquelles se d�placer !
class Chemin {
    bool inaccessible;
    vector<int> chemin;
public:

    Chemin();

    int distance() const noexcept;
    int destination() const noexcept;
    void setInaccessible() noexcept;
    void setInaccessibleIfEmpty() noexcept;
    bool isAccessible() const noexcept;
    bool empty() const noexcept;
    string toString() const noexcept;

    void removeFirst(); // Enl�ve la premi�re case du chemin, utile pour mettre le chemin � jour
    void addFirst(int); // Inverse de removeFirst, place une case suppl�mentaire au d�but du chemin
    void resetChemin(); // R�initialise le chemin
    int getFirst() const; // Renvoie la premi�re case du chemin
    int getLast() const; // Renvoie la derni�re case du chemin
    vector<int> getChemin() const;
};



#endif
