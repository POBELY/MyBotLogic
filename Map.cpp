
#include "Map.h"
#include "MapTile.h"
#include "GameManager.h"
#include "Globals.h"
#include <map>
#include <algorithm>
#include <chrono>
#include <limits>
using namespace std;

Map::Map(const LevelInfo levelInfo) :
    rowCount{ levelInfo.rowCount },
    colCount{ levelInfo.colCount },
    nbTilesDecouvertes{ 0 },
    tiles{ vector<MapTile>{} },
    murs{ map<unsigned int, ObjectInfo>{} },
    fenetres{ map<unsigned int, ObjectInfo>{} },
    portes{ map<unsigned int, ObjectInfo>{} },
    activateurs{ map<unsigned int, ObjectInfo>{} }
{
    // Créer toutes les tiles !
    tiles.reserve(getNbTiles());
    for (int id = 0; id < getNbTiles(); ++id) {
        tiles.push_back(MapTile(id, *this));
    }

    // Mettre à jour les tiles connues
    for (auto tile : levelInfo.tiles) {
        addTile(tile.second);
    }

    // Enregistrer les objets
    for (auto object : levelInfo.objects) {
        addObject(object.second);
    }

    // Mettre à visiter les cases initiales des NPCs
    for (auto pair_npc : levelInfo.npcs) {
       if (tiles[pair_npc.second.tileID].getVoisinsMursNonInspecte().empty()) {
          tiles[pair_npc.second.tileID].setStatut(MapTile::Statut::INSPECTEE);
       }
       else {
          tiles[pair_npc.second.tileID].setStatut(MapTile::Statut::VISITE);
       }
       // Mettre a visitable les voisins accesible connu d'une case visité
       for (auto voisinID : tiles[pair_npc.second.tileID].getVoisinsAccessibles()) {
          if (tiles[voisinID].getStatut() == MapTile::Statut::CONNU) {
             tiles[voisinID].setStatut(MapTile::Statut::VISITABLE);
          }
       }
    }

    // Creer matrice distancesAStar
    distancesAStar = vector<vector<int>>(getNbTiles(), vector<int>(getNbTiles(), -1));
}

bool Map::isInMap(int idTile) const noexcept {
    return idTile >= 0 && idTile < rowCount * colCount;
}

vector<unsigned int> Map::getObjectifs() const noexcept {
    return objectifs;
}

struct Noeud {
    static float coefEvaluation;
    MapTile tile;
    float cout; // La distance calculé depuis le départ
    float evaluation; // La distance estimée à l'arrivée
    float heuristique; // La somme du cout et de l'evaluation
    int idPrecedant;
    Noeud() = default;
    Noeud(MapTile tile, float cout, float evaluation, int idPrecedant)
        : tile{ tile }, cout{ cout }, evaluation{ evaluation }, idPrecedant{ idPrecedant } {
        heuristique = cout + evaluation * coefEvaluation;
    }
    friend bool operator==(const Noeud& g, const Noeud& d) {
        return g.tile.getId() == d.tile.getId();
    }
};
float Noeud::coefEvaluation = 1;

using tile_id = unsigned int;
struct PathNode {
    using tile_id = unsigned int;

    tile_id tile;
    tile_id previous;
    unsigned int cost_so_far;
    unsigned int estimated_total_cost;

    PathNode(tile_id tile, uint32_t cost, uint32_t heuristic)
        : tile{ tile }
        , previous{ std::numeric_limits<tile_id>::max() }
        , cost_so_far{ cost }
        , estimated_total_cost{ heuristic } {

    }

    PathNode(tile_id tile, tile_id previous_tile, uint32_t cost, uint32_t heuristic)
        : tile{ tile }
        , previous{ previous_tile }
        , cost_so_far{ cost }
        , estimated_total_cost{ heuristic } {

    }
};

Chemin get_path(std::vector<PathNode>&& nodes, tile_id end) {
   Chemin path;

   // A path was found
   if (!nodes.empty() && nodes.back().tile == end) {
      PathNode& current_node = nodes.back();

      while (current_node.previous != std::numeric_limits<tile_id>::max()) {
          path.addFirst(current_node.tile);

         auto it = std::find_if(std::begin(nodes), std::end(nodes), [&current_node](const PathNode& node) {
            return node.tile == current_node.previous;
         });

         if (it != std::end(nodes)) {
            current_node = *it;
         }
         else {
            throw 5;
         }
      }
   }

   path.setInaccessibleIfEmpty();

   return path;
}


// Il s'agit de l'algorithme AStar auquel on peut rajouter un coefficiant à l'évaluation pour modifier l'heuristique.
// Par défaut sa valeur est 1. Si on l'augmente l'algorithme ira plus vite au détriment de trouver un chemin optimal.
// Si on le diminue l'algorithme se rapproche de plus en plus d'un parcours en largeur.
Chemin Map::aStar(int depart, int arrivee, float coefEvaluation) noexcept {
    PROFILE_SCOPE("aStar");

    std::vector<PathNode> close_nodes;
    std::vector<PathNode> open_nodes;

    open_nodes.reserve(total_size());

    open_nodes.emplace_back(depart, 0, distanceL2(depart, arrivee));

    while (!open_nodes.empty()) {
        // Smallest node is always the last
        const std::size_t current_node_index = open_nodes.size() - 1;
        const PathNode node = open_nodes.back();
        const MapTile& current_tile = tiles[node.tile];

        // A path is found
        if (node.tile == arrivee) {
            close_nodes.push_back(node);
            break;
        }

        // For each neighbor
        //for (const tile_id neighbor : nodes[node.tile].neighbors) {
        for(const tile_id neighbor : current_tile.getVoisinsAccessibles()) {
            if (neighbor == node.tile) continue; // Skip itself

            const unsigned int cost_to_neighbor = 1;
            const unsigned int neighbor_cost = node.cost_so_far + cost_to_neighbor;

            const auto search_predicate = [neighbor](const PathNode& node) {
                return node.tile == neighbor;
            };

            // Search in open and close lists if the neighbor is already there
            auto closed_it = std::find_if(std::begin(close_nodes), std::end(close_nodes), search_predicate);
            auto open_it = std::find_if(std::begin(open_nodes), std::end(open_nodes), search_predicate);

            // If node is in close list, we may skip it or remove it from the list
            if (closed_it != std::end(close_nodes)) {

                // We didn't find a shorter path to neighbor, skip
                if (closed_it->estimated_total_cost <= neighbor_cost + distanceL2(neighbor, arrivee)) continue;

                // We found a shorter path to neighbor, remove the old one
                std::swap(*closed_it, close_nodes.back());
                close_nodes.pop_back();
            }
            else if (open_it != std::end(open_nodes)) {
                if (open_it->cost_so_far <= neighbor_cost) continue;
            }

            // add the neighbor to the open list
            open_nodes.emplace_back(neighbor, node.tile, neighbor_cost, neighbor_cost + distanceL2(neighbor, arrivee));
        }

        // Add the current node to the close list
        close_nodes.push_back(node);

        // Remove the current node from the open list
        std::swap(open_nodes[current_node_index], open_nodes.back());
        open_nodes.pop_back();

        // Put the smallest node at the end of the open nodes list
        if (!open_nodes.empty()) {
            auto smallest_in_open = std::min_element(std::begin(open_nodes), std::end(open_nodes), [](const PathNode& a, const PathNode& b) {
                return a.estimated_total_cost < b.estimated_total_cost;
            });
            std::swap(*smallest_in_open, open_nodes.back());
        }
    }

    // Mettre a jour la matrice distancesAStar
    // Calculer pour tous les chemins intermédiaires !!! A FAIRE
    Chemin path = get_path(std::move(close_nodes), arrivee);
    vector<int> chemin = path.getChemin();
    (distancesAStar[depart])[arrivee] = chemin.size();
    (distancesAStar[arrivee])[depart] = chemin.size();

    return path;
}

Tile::ETilePosition Map::getDirection(int ind1, int ind2) const noexcept {
    int y = getY(ind1);
    bool pair = (y % 2 == 0);
    if (pair) {
        if (ind2 == ind1 - colCount) {
            return Tile::NE;
        }
        else if (ind2 == ind1 + 1) {
            return Tile::E;
        }
        else if (ind2 == ind1 + colCount) {
            return Tile::SE;
        }
        else if (ind2 == ind1 + colCount - 1) {
            return Tile::SW;
        }
        else if (ind2 == ind1 - 1) {
            return Tile::W;
        }
        else if (ind2 == ind1 - colCount - 1) {
            return Tile::NW;
        }
    }
    else {
        if (ind2 == ind1 - colCount + 1) {
            return Tile::NE;
        }
        else if (ind2 == ind1 + 1) {
            return Tile::E;
        }
        else if (ind2 == ind1 + colCount + 1) {
            return Tile::SE;
        }
        else if (ind2 == ind1 + colCount) {
            return Tile::SW;
        }
        else if (ind2 == ind1 - 1) {
            return Tile::W;
        }
        else if (ind2 == ind1 - colCount) {
            return Tile::NW;
        }
    }

    GameManager::Log("Erreur dans l'appel de getDirection() !");
    return Tile::CENTER;
}

int Map::getAdjacentTileAt(int tileSource, Tile::ETilePosition direction) const noexcept {
    int y = getY(tileSource);
    bool pair = (y % 2 == 0);
    int res;
    switch (direction)
    {
    case Tile::NE:
        if (pair) {
            res = tileSource - colCount;
        }
        else {
            res = tileSource - colCount + 1;
        }
        break;
    case Tile::E:
        res = tileSource + 1;
        break;
    case Tile::SE:
        if (pair) {
            res = tileSource + colCount;
        }
        else {
            res = tileSource + colCount + 1;
        }
        break;
    case Tile::SW:
        if (pair) {
            res = tileSource + colCount - 1;
        }
        else {
            res = tileSource + colCount;
        }
        break;
    case Tile::W:
        res = tileSource - 1;
        break;
    case Tile::NW:
        if (pair) {
            res = tileSource - colCount - 1;
        }
        else {
            res = tileSource - colCount;
        }
        break;
    case Tile::CENTER:
        res = tileSource;
        break;
    default:
        break;
    }

    if (isInMap(res)) {
        return res;
    }
    else {
        //GameManager::Log("La direction demandé dans getAdjacentTileAt n'existe pas !");
        //GameManager::Log("origin = " + to_string(tileSource) + " direction = " + to_string(direction));
        return -1;
    }
}

float Map::distanceL2(int depart, int arrivee) const noexcept {
    int xd = depart % colCount;
    int yd = depart / colCount;
    int xa = arrivee % colCount;
    int ya = arrivee / colCount;
    return (float)sqrt(pow(xd - xa, 2) + pow(yd - ya, 2));
}

int Map::distanceHex(int tile1ID, int tile2ID) const noexcept {
    int ligne1 = tile1ID / colCount;
    int colonne1 = tile1ID % colCount;
    int ligne2 = tile2ID / colCount;
    int colonne2 = tile2ID % colCount;
    int x1 = colonne1 - (ligne1 - ligne1 % 2) / 2;
    int z1 = ligne1;
    int y1 = -x1 - z1;
    int x2 = colonne2 - (ligne2 - ligne2 % 2) / 2;
    int z2 = ligne2;
    int y2 = -x2 - z2;
    return max(max(abs(x1 - x2), abs(y1 - y2)), abs(z1 - z2));
}

int Map::tailleCheminMax() const noexcept {
    return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est déjà dans la map !
void Map::addTile(TileInfo tile) noexcept {
    // On met à jour le nombre de tiles
    ++nbTilesDecouvertes;

    // On la rajoute aux tiles
    tiles[tile.tileID].setTileDecouverte(tile);

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Goal) {
        objectifs.push_back(tile.tileID);
    }

    if (tiles[tile.tileID].getType() == Tile::TileAttribute_Forbidden) {
        for (auto voisin : tiles[tile.tileID].getVoisins()) {
            tiles[voisin].removeAccessible(tile.tileID);
        }
    }

    // Puis on met à jour les voisins de ses voisins ! :D
    for (auto voisin : tiles[tile.tileID].getVoisins()) { // On pourrait parcourir les voisinsVisibles
        // Si ce voisin l'a en voisin mystérieux, on le lui enlève
        tiles[voisin].removeMysterieux(tile.tileID);
    }

    // On le note !
    GameManager::Log("Decouverte de la tile " + to_string(tile.tileID));
}

// Il ne faut pas ajouter un objet qui est déjà dans la map !
void Map::addObject(ObjectInfo object) noexcept {
    int voisin1 = object.tileID;
    int voisin2 = getAdjacentTileAt(object.tileID, object.position);

    // On ajoute notre objet à l'ensemble de nos objets
    if (object.objectTypes.find(Object::ObjectType_Wall) != object.objectTypes.end()) {
        // Fenetre
        if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
            fenetres[object.objectID] = object;
            if (isInMap(voisin1))
                tiles[voisin1].removeAccessible(voisin2);
            if (isInMap(voisin2))
                tiles[voisin2].removeAccessible(voisin1);
        // Mur
        }
        else {
           if (hadInteract(object.objectID)) {
              if (isInMap(voisin1)) {
                 tiles[voisin1].removeMurNonInspectee(object.objectID);
              }
              if (isInMap(voisin2)) {
                 tiles[voisin2].removeMurNonInspectee(object.objectID);
              }
           }

            murs[object.objectID] = object;
            if (isInMap(voisin1)) {
                tiles[voisin1].removeMysterieux(voisin2);
                tiles[voisin1].removeAccessible(voisin2);
                tiles[voisin1].removeVisible(voisin2);
                tiles[voisin1].addMur(object.objectID);
            }
            if (isInMap(voisin2)) {
                tiles[voisin2].removeMysterieux(voisin1);
                tiles[voisin2].removeAccessible(voisin1);
                tiles[voisin2].removeVisible(voisin1);
                tiles[voisin2].addMur(object.objectID);
            }
        }
    }
    if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
        portes[object.objectID] = object;
        //Porte Ferme
        if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
            // Porte Fenetre
            if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
                if (isInMap(voisin1))
                    tiles[voisin1].removeAccessible(voisin2);
                if (isInMap(voisin2))
                    tiles[voisin2].removeAccessible(voisin1);
            // Porte
            } else {
               // Porte isolée
               if (object.connectedTo.empty()) {
                  if (find(isolatedClosedDoors.begin(), isolatedClosedDoors.end(), object.objectID) == isolatedClosedDoors.end()) {
                     isolatedClosedDoors.push_back(object.objectID);
                  }
                  if (hadInteract(object.objectID) && (murs.find(object.objectID) != murs.end())) {
                     // Supprimer le mur du modèle
                     murs.erase(murs.find(object.objectID));
                     if (isInMap(voisin1)) {
                        tiles[voisin1].removeMurNonInspectee(object.objectID);
                        tiles[voisin1].removeMur(object.objectID);
                     }
                     if (isInMap(voisin2)) {
                        tiles[voisin2].removeMurNonInspectee(object.objectID);
                        tiles[voisin2].removeMur(object.objectID);
                     }
                  } else {
                     
                     if (isInMap(voisin1)) {
                        tiles[voisin1].removeAccessible(voisin2);
                        tiles[voisin1].removeVisible(voisin2);
                     }
                     if (isInMap(voisin2)) {
                        tiles[voisin2].removeAccessible(voisin1);
                        tiles[voisin2].removeVisible(voisin1);
                     }
                  }
               
               // Porte Connectée
               } else {
                  if (isInMap(voisin1)) {
                     tiles[voisin1].removeAccessible(voisin2);
                     tiles[voisin1].removeVisible(voisin2);
                  }
                  if (isInMap(voisin2)) {
                     tiles[voisin2].removeAccessible(voisin1);
                     tiles[voisin2].removeVisible(voisin1);
                  }
               }
            }
        // Porte ouverte
        } else {
           // Si on vient d'ouvrir la porte, on la suprime des portes isolés fermés
           if (hadInteract(object.objectID)) {
              isolatedClosedDoors.erase(find(isolatedClosedDoors.begin(), isolatedClosedDoors.end(), object.objectID));
           }
           // Si la porte est ouverte on est accessible ET visible ! =)
           vector<int> voisins1Accessibles = tiles[voisin1].getVoisinsAccessibles();
           if (find(voisins1Accessibles.begin(), voisins1Accessibles.end(), voisin2) == voisins1Accessibles.end()) {
              tiles[voisin1].addVoisinAccessible(voisin2);
              tiles[voisin2].addVoisinAccessible(voisin1);
           }
           vector<int> voisins1Visibles = tiles[voisin1].getVoisinsVisibles();
           if (find(voisins1Visibles.begin(), voisins1Visibles.end(), voisin2) == voisins1Visibles.end()) {
              tiles[voisin1].addVoisinVisible(voisin2);
              tiles[voisin2].addVoisinVisible(voisin1);
           }
        }
    }
    if (object.objectTypes.find(Object::ObjectType_PressurePlate) != object.objectTypes.end()) {
        activateurs[object.objectID] = object;
        tiles[voisin1].setActivateur(object.objectID);
        // prout !
    }

    // On le note !
    GameManager::Log("Decouverte de l'objet " + to_string(object.objectID) + " sur la tuile " + to_string(object.tileID) + " orienté en " + to_string(object.position));
}

void Map::addInteractObject(int objectID) {
   interactObjects.push_back(objectID);
}

void Map::viderInteractObjects() {
   interactObjects = {};
}

int Map::getX(int id) const noexcept {
    return id % colCount;
}
int Map::getY(int id) const noexcept {
    return id / colCount;
}

vector<int> Map::getVoisins(int id) const noexcept {
    vector<int> voisins;
    int x = getX(id);
    int y = getY(id);
    int indice;
    if (y % 2 == 0) { // Ligne paire
        // NE
        indice = id - colCount;
        if (isInMap(indice) && y > 0) {
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (isInMap(indice) && x < colCount - 1) {
            voisins.push_back(indice);
        }
        // SE
        indice = id + colCount;
        if (isInMap(indice) && y < rowCount - 1) {
            voisins.push_back(indice);
        }
        // SW
        indice = id + colCount - 1;
        if (isInMap(indice) && y < rowCount - 1 && x > 0) {
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = id - colCount - 1;
        if (isInMap(indice) && y > 0 && x > 0) {
            voisins.push_back(indice);
        }

    }
    else { // Ligne impaire !
     // NE
        indice = id - colCount + 1;
        if (isInMap(indice) && x < colCount - 1) {
            voisins.push_back(indice);
        }
        // E
        indice = id + 1;
        if (isInMap(indice) && x < colCount - 1) {
            voisins.push_back(indice);
        }
        // SE
        indice = id + colCount + 1;
        if (isInMap(indice) && x < colCount - 1 && y < rowCount - 1) {
            voisins.push_back(indice);
        }
        // SW
        indice = id + colCount;
        if (isInMap(indice) && y < rowCount - 1) {
            voisins.push_back(indice);
        }
        // W
        indice = id - 1;
        if (isInMap(indice) && x > 0) {
            voisins.push_back(indice);
        }
        // NW
        indice = id - colCount;
        if (isInMap(indice)) { // Pas de conditions, c'est marrant ! =)
            voisins.push_back(indice);
        }
    }

    return voisins;
}

int Map::getRowCount() const noexcept {
    return rowCount;
}

int Map::getColCount() const noexcept {
    return colCount;
}

int Map::getNbTiles() const noexcept {
    return getRowCount() * getColCount();
}

int Map::getNbTilesDecouvertes() const noexcept {
    return nbTilesDecouvertes;
}

MapTile& Map::getTile(int id) {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

const MapTile& Map::getTile(int id) const {
    if (id < 0 || id >= getNbTiles())
        throw tile_inexistante{};
    return tiles[id];
}

vector<unsigned int> Map::getObjectifs() {
    return objectifs;
}

map<unsigned int, ObjectInfo> Map::getMurs() {
    return murs;
}

map<unsigned int, ObjectInfo> Map::getPortes() {
    return portes;
}

map<unsigned int, ObjectInfo> Map::getFenetres() {
    return fenetres;
}

map<unsigned int, ObjectInfo> Map::getActivateurs() {
    return activateurs;
}

vector<int> Map::getInteractObjects() {
   return interactObjects;
}

vector<int> Map::getIsolatedClosedDoors() {
   return isolatedClosedDoors;
}

int Map::getDistance(int tile1, int tile2) {
   int dist = getDistanceAStar(tile1,tile2);
   if (dist != -1) {
      return dist;
   } else {
      aStar(tile1, tile2);
      return getDistanceAStar(tile1, tile2);
   }
}

int Map::getDistanceAStar(int tile1, int tile2) {
   return (distancesAStar[tile1])[tile2];
}

bool Map::objectExist(int objet) {
   return murs.find(objet) != murs.end()
          || fenetres.find(objet) != fenetres.end()
          || activateurs.find(objet) != activateurs.end();
        //|| portes.find(objet) != portes.end() // On regarde les portes à tous les tours
}

bool Map::hadInteract(int objet) {
   return find(interactObjects.begin(), interactObjects.end(), objet) != interactObjects.end(); // Si on a interagit avec cet objet au tout précédent, on le regarde à nouveau
}
