#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <set>

struct estado {
  int fila;
  int columna;
  int orientacion;
};

inline bool operator<(const estado& lhs, const estado& rhs) {
   return lhs.fila < rhs.fila;
}

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
      //añadidos nivel 2s

    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}


  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino, ultimoEstado, banio, deporte;
    list<Action> plan;
    Action ultimaAccion;
    bool hayplan;
    bool hayRecarga = false;
    bool bikini =false;
    bool hayBikini = false;
    bool zapatillas = false;
    bool hayZapatillas =false;
    bool buscaBikini = false;
    bool buscaZapas = false;
    bool recargado = false;
    int costePlan = 0;
    int contador = 0;
    int tiempoMax = 0;
    set<estado> recargas;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan, bool bik, bool zap, int elTiempo);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CostoUniforme_AdaptadoNivel4(const estado &origen, const estado &destino, list<Action> &plan, bool bik, bool zap, int elTiempo);

    int CalculaCostes(unsigned char casilla, bool bikini, bool zapatillas, int coste);
    int CalculaCostesMapaGrande(unsigned char casilla, bool bikini, bool zapatillas, int coste);
    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
    void mapeo(estado st, vector<unsigned char> sensor);
    int indentificaEsquina(estado st);

};

#endif
