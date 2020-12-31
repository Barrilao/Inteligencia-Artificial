#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <stack>
#include <queue>
#include <ctime>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
    //think del nivel 1
    if (sensores.nivel != 4){
        if(!hayplan){
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            hayplan = pathFinding(sensores.nivel, actual, destino, plan,false,false,25);
        }
        Action sigAccion;
        if (hayplan and plan.size()>0){
            sigAccion = plan.front();
            plan.erase(plan.begin());
        }
        else{
            cout << "No hay plan";
        }
        return sigAccion;
    }

    //think del nivel 2

    else{
        //tiempo máximo en función del tamaño del mapa
        if(mapaResultado.size()>75)
            tiempoMax=15;
        else
            tiempoMax=20;

        //en el caso de que quede poco tiempo y suficiente bateria hace un camino por anchura para apurar sus recursos
        if(sensores.bateria>600 && contador>2750&&!hayplan){
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            mapeo(actual,sensores.terreno);
            hayplan = pathFinding(2, actual, destino, plan, bikini, zapatillas,tiempoMax);
            buscaBikini = false;
            buscaZapas = false;
        }

        //Se activa si se está agotando la batería, sabe donde están las casillas de recarga y hay tiempo para recargar
        else if(sensores.bateria < 850 && contador < 2500 && hayRecarga && !recargado){
            cout << "Recarga necesaria\n";
            plan.clear();
            int coste=100000;
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            set<estado>::iterator estadosIterador;
            estado enJuego, salida;
            for(estadosIterador=recargas.begin();estadosIterador!=recargas.end();++estadosIterador){
                enJuego=*estadosIterador;
                destino.fila=enJuego.fila;
                destino.columna=enJuego.columna;
                hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini, zapatillas,tiempoMax);
                if(coste>costePlan){
                    coste=costePlan;
                    salida=enJuego;
                }
            }
            destino.fila=salida.fila;
            destino.columna=salida.columna;
            hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini, zapatillas,tiempoMax);
            recargado = true;
            buscaBikini = false;
            buscaZapas = false;
        }

        //Se activa si ve un bikini y no lo tiene equipado
        else if(hayBikini && !bikini && !buscaBikini){
            cout << "Ruta hacia bikini por proximidad\n";
            plan.clear();
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            destino.fila=banio.fila;
            destino.columna=banio.columna;
            hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini, zapatillas,tiempoMax);
            buscaBikini = true;
        }
        //Se activa si ve unas zapatillas y no las tiene equipadas
        else if(hayZapatillas && !zapatillas && !buscaZapas){
            cout << "Ruta hacia zapatillas por proximidad\n";
            plan.clear();
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            destino.fila=deporte.fila;
            destino.columna=deporte.columna;
            hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini, zapatillas,tiempoMax);
            buscaZapas = true;
        }

        //Si no hay plan busca un plan
        if(!hayplan){
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
            destino.fila = sensores.destinoF;
            destino.columna = sensores.destinoC;
            mapeo(actual,sensores.terreno);
            hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini, zapatillas,tiempoMax);
            recargado = false;
            buscaBikini = false;
            buscaZapas = false;
        }
        Action sigAccion;
        if (hayplan and plan.size()>0){
            sigAccion = plan.front();
            if(sensores.terreno[2]=='M' && sigAccion == actFORWARD||sensores.superficie[2]=='a' && sigAccion == actFORWARD && tiempoMax==20){
                plan.clear();
                actual.fila = sensores.posF;
                actual.columna = sensores.posC;
                actual.orientacion = sensores.sentido;
                mapeo(actual,sensores.terreno);
                hayplan = pathFinding(sensores.nivel, actual, destino, plan, bikini,zapatillas,tiempoMax);
                if(sensores.superficie[2]=='a' && tiempoMax==20){
                    cout << "Ha colisionado contra un aldeano\n";
                    sigAccion = actIDLE;
                }
                else{
                    cout << "Ha colisionado contra un muro\n";
                    sigAccion = plan.front();
                }
            }
            //evitar caida por precipicio
            if(sensores.terreno[2] == 'P' && sigAccion == actFORWARD){
                cout << "Muerte evitada \n";
                plan.clear();
                hayplan = false;
                sigAccion = actIDLE;
                recargado = false;
            }

            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;

            //evitar agua si tiene menos de la mitad de la bateria o no tiene el bikini
            if((sensores.terreno[2]=='A'&& sigAccion == actFORWARD && !bikini && buscaBikini == false
            && buscaZapas == false)||(sensores.terreno[2]=='A'&& sigAccion == actFORWARD && bikini && buscaBikini == false
            && buscaZapas == false && sensores.bateria < 1500 && sensores.terreno[0]!='A')){
                if((ultimoEstado.columna!=actual.columna || ultimoEstado.fila!=actual.fila ||
                   ultimoEstado.orientacion!=actual.orientacion) && ultimaAccion!=actIDLE){
                    cout << "Evitar chapuzon sin bikini \n";
                    plan.clear();
                    hayplan = false;
                    sigAccion = actIDLE;
                    recargado = false;
                }
            }
            //evitar bosque si no tiene zapatillas
            else if(sensores.terreno[2]=='B'&& sigAccion == actFORWARD && !zapatillas && buscaBikini == false
            && buscaZapas == false && sensores.terreno[0]!='B'){
                if((ultimoEstado.columna!=actual.columna || ultimoEstado.fila!=actual.fila ||
                   ultimoEstado.orientacion!=actual.orientacion) && ultimaAccion!=actIDLE){
                    cout << "Evitar ruta por el bosque sin zapatillas \n";
                    plan.clear();
                    hayplan = false;
                    sigAccion = actIDLE;
                    recargado = false;
                }
            }
            plan.erase(plan.begin());
            mapeo(actual,sensores.terreno);
        }
        else{
            cout << "No hay plan \n";
            hayplan=false;
            sigAccion = actIDLE;
            recargado = false;
        }

        //si hay algun objeto advierte de que esta equipado
        if(sensores.terreno[0]=='K')
            bikini=true;
        else if(sensores.terreno[0]=='D')
            zapatillas=true;

        ultimaAccion = sigAccion;
        ultimoEstado=actual;
        contador++;

        //si estamos en la casilla de recarga debemos recargar
        if(sensores.terreno[0]=='X'&&((sensores.bateria<2400&&contador<2000)||(sensores.bateria<1200&&contador<2500))){
            cout << "Recargando \n";
            hayplan = true;
            sigAccion = actIDLE;
            recargado =true;
        }
        else if (sensores.terreno[0]=='X'&&sensores.bateria>2400){
            recargado = false;
        }
        return sigAccion;
    }

}

void ComportamientoJugador::mapeo(estado st, vector<unsigned char> sensor){
	int i, j, k, fil = st.fila, col = st.columna;
	mapaResultado[fil][col] = sensor[0];
    estado melancolico;
    if (mapaResultado[fil][col]=='X'){
        melancolico.fila=fil;
        melancolico.columna=col;
        recargas.insert(melancolico);
        hayRecarga=true;
    }
    else if (mapaResultado[fil][col]=='K') {
        banio.fila=fil;
        banio.columna=col;
        hayBikini = true;
    }
    else if (mapaResultado[fil][col]=='D') {
        deporte.fila=fil;
        deporte.columna=col;
        hayZapatillas = true;
    }

	switch (st.orientacion) {
		case 0: fil--;col--; break;
		case 1: col++;fil--; break;
		case 2: fil++;col++; break;
		case 3: col--;fil++; break;
	}
	for(i = 1, j = fil, k = col; i < 4; ++i){
		mapaResultado[j][k] = sensor[i];
        if (mapaResultado[j][k]=='X'){
            melancolico.fila=j;
            melancolico.columna=k;
            recargas.insert(melancolico);
            hayRecarga=true;
        }
        else if (mapaResultado[j][k]=='K') {
            banio.fila=j;
            banio.columna=k;
            hayBikini = true;
        }
        else if (mapaResultado[j][k]=='D') {
            deporte.fila=j;
            deporte.columna=k;
            hayZapatillas = true;
        }

		switch (st.orientacion) {
			case 0: k++; break;
			case 1: j++; break;
			case 2: k--; break;
			case 3: j--; break;
		}
	}

	switch (st.orientacion) {
		case 0: fil--;col--; break;
		case 1: col++;fil--; break;
		case 2: fil++;col++; break;
		case 3: col--;fil++; break;
	}

	for(i = 4, j = fil, k = col; i < 9; ++i){
		mapaResultado[j][k] = sensor[i];
        if (mapaResultado[j][k]=='X'){
            melancolico.fila=j;
            melancolico.columna=k;
            recargas.insert(melancolico);
            hayRecarga=true;
        }
        else if (mapaResultado[j][k]=='K') {
            banio.fila=j;
            banio.columna=k;
            hayBikini = true;
        }
        else if (mapaResultado[j][k]=='D') {
            deporte.fila=j;
            deporte.columna=k;
            hayZapatillas = true;
        }
		switch (st.orientacion) {
			case 0: k++; break;
			case 1: j++; break;
			case 2: k--; break;
			case 3: j--; break;
		}
	}

	switch (st.orientacion) {
		case 0: fil--;col--; break;
		case 1: col++;fil--; break;
		case 2: fil++;col++; break;
		case 3: col--;fil++; break;
	}

	for(i = 9, j = fil, k = col; i < 16; ++i){
		mapaResultado[j][k] = sensor[i];
        if (mapaResultado[j][k]=='X'){
            melancolico.fila=j;
            melancolico.columna=k;
            recargas.insert(melancolico);
            hayRecarga=true;
        }
        else if (mapaResultado[j][k]=='K') {
            banio.fila=j;
            banio.columna=k;
            hayBikini = true;
        }
        else if (mapaResultado[j][k]=='D') {
            deporte.fila=j;
            deporte.columna=k;
            hayZapatillas = true;
        }
		switch (st.orientacion) {
			case 0: k++; break;
			case 1: j++; break;
			case 2: k--; break;
			case 3: j--; break;
		}
	}

}



// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan, bool bik, bool zap, int elTiempo){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
						return pathFinding_CostoUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						return pathFinding_CostoUniforme_AdaptadoNivel4(origen,destino,plan,bik,zap,elTiempo);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

struct nodo{
	estado st;
	list<Action> secuencia;
	int coste=0;
	bool bikini=false;
	bool zapatillas=false;
};

//Responde a si dos estados son diferentes

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

//Responde a si un coste es mayor que otro

struct ComparaCostes{
	bool operator()(const nodo &a, const nodo &n) const{
		if ((a.coste < n.coste )  or (a.coste == n.coste and a.st.fila > n.st.fila) or
				(a.coste == n.coste and a.st.fila == n.st.fila and a.st.columna > n.st.columna) or
				(a.coste == n.coste and a.st.fila == n.st.fila and a.st.columna == n.st.columna and a.st.orientacion > n.st.orientacion))
			return true;
		else
			return false;
	}
};

//Calcula el coste de un nodo

int ComportamientoJugador::CalculaCostes(unsigned char casilla, bool bikini, bool zapatillas, int coste){
    int costeFinal=0;

    if (bikini && zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 5;
                                                break;
            case 'A': costeFinal = coste + 10;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    else if (bikini && !zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 50;
                                                break;
            case 'A': costeFinal = coste + 10;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    else if (!bikini && zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 5;
                                                break;
            case 'A': costeFinal = coste + 100;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }

    else{
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 50;
                                                break;
            case 'A': costeFinal = coste + 100;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    return costeFinal;

}

int ComportamientoJugador::CalculaCostesMapaGrande(unsigned char casilla, bool bikini, bool zapatillas, int coste){
    int costeFinal=0;

    if (bikini && zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 5;
                                                break;
            case 'A': costeFinal = coste + 10;
                                                break;
            case '?': costeFinal = coste + 2;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    else if (bikini && !zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 50;
                                                break;
            case 'A': costeFinal = coste + 10;
                                                break;
            case '?': costeFinal = coste + 2;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    else if (!bikini && zapatillas){
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 5;
                                                break;
            case 'A': costeFinal = coste + 100;
                                                break;
            case '?': costeFinal = coste + 2;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }

    else{
        switch (casilla) {
            case 'T': costeFinal = coste + 2;
                                                break;
            case 'B': costeFinal = coste + 50;
                                                break;
            case 'A': costeFinal = coste + 100;
                                                break;
            case '?': costeFinal = coste + 2;
                                                break;
            default : costeFinal = coste + 1;
                                                break;
        }
    }
    return costeFinal;

}



// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

// Implementación de la búsqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
			//No almacenar estados que ya tengamos en generados
            while(generados.find(current.st)!=generados.end()&& !cola.empty())
            {
                cola.pop();
                current = cola.front();
            }
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


// Implementación de la búsqueda con costo uniforme.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	multiset<nodo,ComparaCostes> lista;											// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	lista.insert(current);

  while (!lista.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		lista.erase(lista.begin());
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
            unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
            hijoTurnR.coste = CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
			hijoTurnR.secuencia.push_back(actTURN_R);
			lista.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
            unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
            hijoTurnL.coste = CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
			hijoTurnL.secuencia.push_back(actTURN_L);
			lista.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
				hijoForward.coste= CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
                switch(casilla){
                    case 'K': hijoForward.bikini=true;
                                            break;
                    case 'D': hijoForward.zapatillas=true;
                                            break;
                    default : break;
                }
                lista.insert(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!lista.empty()){
			current = *(lista.begin());
			//No almacenar estados que ya tengamos en generados
            while(generados.find(current.st)!=generados.end()&& !lista.empty())
            {
                lista.erase(lista.begin());
                current = *(lista.begin());
            }
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


bool ComportamientoJugador::pathFinding_CostoUniforme_AdaptadoNivel4(const estado &origen, const estado &destino, list<Action> &plan, bool bik, bool zap, int elTiempo) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	multiset<nodo,ComparaCostes> lista;											// Lista de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();
    current.bikini=bik;
    current.zapatillas=zap;

	lista.insert(current);
    double time=0;
	unsigned t0, t1;
	t0=clock();
  while (!lista.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)&&time<elTiempo){

		lista.erase(lista.begin());
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
            unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
            if(elTiempo==25){
                hijoTurnR.coste = CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
            }
            else{
                hijoTurnR.coste = CalculaCostesMapaGrande(casilla,current.bikini,current.zapatillas,current.coste);
            }
			hijoTurnR.secuencia.push_back(actTURN_R);
			lista.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
            unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
            if(elTiempo==25){
                hijoTurnL.coste = CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
            }
            else{
                hijoTurnL.coste = CalculaCostesMapaGrande(casilla,current.bikini,current.zapatillas,current.coste);
            }
			hijoTurnL.secuencia.push_back(actTURN_L);
			lista.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				unsigned char casilla = mapaResultado[current.st.fila][current.st.columna];
				if(elTiempo==25){
                    hijoForward.coste= CalculaCostes(casilla,current.bikini,current.zapatillas,current.coste);
				}
				else{
                    hijoForward.coste= CalculaCostesMapaGrande(casilla,current.bikini,current.zapatillas,current.coste);
				}
                switch(casilla){
                    case 'K': hijoForward.bikini=true;
                                            break;
                    case 'D': hijoForward.zapatillas=true;
                                            break;
                    default : break;
                }
                lista.insert(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!lista.empty()){
			current = *(lista.begin());
			//No almacenar estados que ya tengamos en generados
			while(generados.find(current.st)!=generados.end()&& !lista.empty())
            {
                lista.erase(lista.begin());
                current = *(lista.begin());
            }
		}
		t1 = clock();
		time = (double(t1-t0)/CLOCKS_PER_SEC);

	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
        cout << "Cargando el plan parcial\n";
        plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}


	return false;
}




// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
