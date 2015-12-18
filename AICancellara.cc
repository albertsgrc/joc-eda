#include"Player.hh"
#include <queue>
#include <stack>
#include <map>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <cmath>
using namespace std;
#define PLAYER_NAME Cancellara
#define TORN quin_torn()
#define JO qui_soc()
#define TORNS torns_restants()
#define RANDOM probabilitat(0.5)

// Orientacions heli
const int SUD = 0;
const int EST = 1;
const int NORD = 2;
const int OEST = 3;
const int NO_GIRIS = -1;
// Altres
const int NEQUIPS = 4;
const int A_H = 5; // Àrea que ocupa l'helicòpter
const int NCAS_H = 5*5;
const int SOLDATS_MAX = 80; // Nombre de soldats màxims
const int INFINIT = 1e9;
// Valor marca
const int MARCA = -1;
// Objectes
const int DE_NINGU = -1;
// Direccions
const int EQUIVALENTS = 0;
const int PERPENDICULAR = 1;
const int OPOSADES = 2;
struct Dir {
int x, y;

Dir() {}
Dir(int x, int y) : x(x), y(y) {}
Dir(const Dir& d) : x(d.x), y(d.y) {}
Dir(int orientacioHeli) {
    switch (orientacioHeli) {
        case SUD: x = 1; y = 0; break;
        case NORD: x = -1; y = 0; break;
        case EST: x = 0; y = 1; break;
        case OEST: x = 0; y = -1; break;
        default:; //assert(false);
    }
}

inline void operator=(const Dir& d) {
    x = d.x; y = d.y;
}

inline Dir operator +=(const Dir& d) {
    x += d.x;
    y += d.y;
    return *this;
}

inline Dir operator+(const Dir& p) const {
    Dir res = *this;
    res += p;
    return res;
}

inline Dir operator -=(const Dir& d) {
    x -= d.x;
    y -= d.y;
    return *this;
}

inline Dir operator-(const Dir& p) const {
    Dir res = *this;
    res -= p;
    return res;
}

inline bool operator!=(const Dir& d) const {
    return x != d.x or y != d.y;
}

inline bool operator==(const Dir& d) const {
    return x == d.x and y == d.y; 
}

inline int prodEsc(const Dir& d) const {
    return d.x*x  + d.y*y;
}

inline int norma() const {
    return sqrt(x*x + y*y);
}

inline double angleRelatiu(const Dir& d) const {
	if ((d.x != 0 or d.y != 0) and (y != 0 or x != 0)) 
 		return (atan2(d.y, d.x) - atan2(y, x))*180.0/M_PI;
 	else //assert(false);
 	return 0;
}

inline double angle(const Dir& d) const {
	if (norma() != 0 and d.norma() != 0) {
		return acos(prodEsc(d)/(norma()*d.norma()))*180.0/M_PI;
	}
	else //assert(false);
	return 0;
}

inline int direccioRel(const Dir& d) const {
    int angle = this->angleRelatiu(d);
    switch (angle) {
    	case 135: case 45: case -270: case -315: case -225: case 90: 
    	return CONTRA_RELLOTGE;
        case -45: case -90: case -135: case 225: case 315: case 270: 
        return RELLOTGE;
   		case -180: case 180:
   		return OPOSADES;
        case 0: return EQUIVALENTS;
        default:; //assert(false);
    }
} 

inline bool perpendicular(const Dir& d) const {
    return prodEsc(d) == 0;
}

inline bool oposades(const Dir& d) const {
	return this->direccioRel(d) == OPOSADES;
}

inline bool equivalents(const Dir& d) const {
	return this->direccioRel(d) == EQUIVALENTS;
}

inline bool paraleles(const Dir& d) const {
	return this->equivalents(d) or this->oposades(d); 
}

inline bool contraRellotge(const Dir& d) const {
	return this->direccioRel(d) == CONTRA_RELLOTGE;
}

inline bool rellotge(const Dir& d) const {
	return this->direccioRel(d) == RELLOTGE;
}

void escriu() const {
    cerr << "x: " << x << " y: " << y << endl;
}

inline bool validaHe() const {
	return not (x and y);
}
};

struct Pos {
int x, y;

Pos() {}
Pos(const Posicio& p) :  x(p.x), y(p.y) {}
Pos(int x, int y) : x(x), y(y) {}

inline void operator=(const Posicio& p) {
    x = p.x; y = p.y;
}

inline bool operator!=(const Pos& p) const {
    return x != p.x or y != p.y;
}

inline bool operator==(const Pos& a) const {
    return x == a.x and y == a.y; 
}

inline Pos operator +=(const Pos& p) {
    x += p.x;
    y += p.y;
    return *this;
}

inline Pos operator+(const Pos& p) const {
    Pos res = *this;
    res += p;
    return res;
}

inline Pos operator-(const Pos& a) const {
    return Pos(x - a.x, y - a.y);
}

inline int distancia(const Pos& p) const {
	return max(abs(p.x - x), abs(p.y - y));
}

inline bool valida() const {
    return x >= 0 and x < MAX and y >= 0 and y < MAX;
}

void escriu() const {
    cerr << "x: " << x << " y: " << y << endl;
}

};

Dir director(const Pos&i, const Pos&d) {
	Dir res(d.x - i.x, d.y - i.y);
	if (res.x == -2 or res.x == 2) res.x /= 2;
	if (res.y == -2 or res.y == 2) res.y /= 2;
	return res;
}

Pos operator+(const Dir& d, const Pos& p) {
	return Pos(d.x + p.x, d.y + p.y);
}

inline Pos operator+(const Pos& p, const Dir& d) {
	return Pos(d.x + p.x, d.y + p.y);
}

struct ResCami {
Dir d;
Dir d2; // Només per helicòpters
Dir dant; // Només per helicòpters
Pos p;
int dist;
int cost;
int ecost;
bool trobat;

ResCami() {}
ResCami(const Dir& d, const Dir& d2, const Dir& dant, const Pos& p, int dist, int cost, 
		bool trobat) :
		d(d), d2(d2), dant(dant), p(p), dist(dist), cost(cost), trobat(trobat) {}
ResCami(const Dir& d, const Dir& d2, const Dir& dant, const Pos& p, int dist, int cost) :
		d(d), d2(d2), dant(dant), p(p), dist(dist), cost(cost) {}
ResCami(const Dir& d, const Dir& d2, const Dir& dant, const Pos& p, int dist, int cost, int ecost) :
		d(d), d2(d2), dant(dant), p(p), dist(dist), cost(cost), ecost(ecost) {}
ResCami(const Dir& d, const Dir& d2, const Pos& p, int dist, int cost, 
		bool trobat) :
		d(d), d2(d2), p(p), dist(dist), cost(cost), trobat(trobat) {}
ResCami(const Dir& d, const Dir& d2, const Pos& p, int dist, bool trobat) :
		d(d), d2(d2), p(p), dist(dist), trobat(trobat) {}
ResCami(const Dir& d, const Pos& p, int dist, int cost, bool trobat) :
		d(d), p(p), dist(dist), cost(cost), trobat(trobat) {}
ResCami(const Dir& d, const Dir& d2, const Pos& p, int dist, int cost) :
		d(d), d2(d2), p(p), dist(dist), cost(cost) {}
ResCami(const Dir& d, const Pos& p, int dist, int cost) :
		d(d), p(p), dist(dist), cost(cost) {}
ResCami(const Dir& d, const Pos& p, int dist, int cost, int ecost) :
		d(d), p(p), dist(dist), cost(cost), ecost(ecost) {}
ResCami(bool trobat) : trobat(trobat) {}
ResCami(const ResCami& rc) : d(rc.d), d2(rc.d2), dant(rc.dant), p(rc.p), dist(rc.dist),
                             cost(rc.cost), ecost(rc.ecost), trobat(rc.trobat) {}

void operator=(const ResCami& rc) {
	d = rc.d;
	d2 = rc.d2;
	dant = rc.dant;
	p = rc.p;
	dist = rc.dist;
	cost = rc.cost;
	ecost = rc.ecost;
	trobat = rc.trobat;
}

bool operator<(const ResCami& c) const {
	return cost > c.cost;
}
};

struct Soldat {
int id, equip, vida;
int nTorns;
bool dirigit;
bool camper;
Pos camp;
Pos p;
Dir d;

int densEm; // Densitat d'enemics, es calcula com sum(nsold*d)
int densMeus; // Densitat de soldats aliats, es calcula com sum(nsold*d)
// Densitat de punts dels objectius propers. Es calcula com sum(npunts*d)
int densPunts;
int densObst; // Densitat d'obstacles adjacents al soldat
int nObstAdj; // Número d'obstacles adjacens al soldat

bool incDensEm;
bool incDensMeus;

Soldat() {}
Soldat(const Soldat& s) :
	   id(s.id), equip(s.equip), vida(s.vida), nTorns(s.nTorns), 
	   dirigit(s.dirigit), camper(s.camper), camp(s.camp), p(s.p), d(s.d), densEm(s.densEm), 
	   densMeus(s.densMeus), densPunts(s.densPunts),
	   densObst(s.densObst), nObstAdj(s.nObstAdj), incDensEm(s.incDensEm),
	   incDensMeus(s.incDensMeus) {}
Soldat(int id, int equip, int vida, int nTorns, bool dirigit, bool camper,
	   const Pos& camp, const Pos& p, const Dir& d, int densEm, int densMeus,
	   int densPunts, int densObst, int nObstAdj, int incDensEm, int incDensMeus) :
	   id(id), equip(equip), vida(vida), nTorns(nTorns), 
	   dirigit(dirigit), camper(camper), camp(camp), p(p), d(d), densEm(densEm), 
	   densMeus(densMeus), densPunts(densPunts),
	   densObst(densObst), nObstAdj(nObstAdj), incDensEm(incDensEm),
	   incDensMeus(incDensMeus) {}

inline void operator=(const Soldat& s) {
    id = s.id;
    equip = s.equip;
    vida = s.vida;
    nTorns = s.nTorns;
    dirigit = s.dirigit;
    camper = s.camper;
    camp = s.camp;
    p = s.p;
    d = s.d;
    densEm = s.densEm;
    densMeus = s.densMeus;
    densPunts = s.densPunts;
    densObst = s.densObst; 
    nObstAdj = s.nObstAdj;
   	incDensEm = s.incDensEm;
    incDensMeus = s.incDensMeus;
}
};

struct Heli {
int id, equip, orie, tNapalm;
bool dirigit;
Pos p;
Pos desti;
Dir d;
VE paracas;

Heli() {}
Heli(const Heli& h) : id(h.id), equip(h.equip), orie(h.orie), 
					  tNapalm(h.tNapalm), dirigit(h.dirigit),
					  p(h.p), desti(h.desti), d(h.d), paracas(h.paracas) {}
Heli(int id, int equip, int orie, int tNapalm, bool dirigit, const Pos& p,
	 const Pos& desti, const Dir& d, const VE&paracas) : id(id), equip(equip), orie(orie), 
					  tNapalm(tNapalm), dirigit(dirigit),
					  p(p), desti(desti), d(d), paracas(paracas) {}

void operator=(const Heli& h) {
	id = h.id;
	equip = h.equip;
	orie = h.orie; 
	tNapalm = h.tNapalm;
	dirigit = h.dirigit;
	p = h.p;
	desti = h.desti;
	d = h.d;
	paracas = h.paracas;
}
};

struct Objectiu {
int equip;
Pos p;
int valor; 

int densEm; // Densitat d'enemics, es calcula com sum(nsold*d)
int densMeus; // Densitat de soldats aliats, es calcula com sum(nsold*d)
// Densitat de punts dels objectius propers. Es calcula com sum(npunts*d)
int densPunts;
int densObst; // Densitat d'obstacles adjacents a l'objectiu
int nObstAdj; // Número d'obstacles adjacens a l'objectiu

Objectiu() {}
Objectiu(const Objectiu& o) : equip(o.equip), p(o.p), valor(o.valor),
							  densEm(o.densEm), densMeus(o.densMeus),
							  densPunts(o.densPunts), densObst(o.densObst),
							  nObstAdj(o.nObstAdj) {}
Objectiu(int equip, const Pos& p, int valor, int densEm, int densMeus,
		 int densPunts, int densObst, int nObstAdj) : equip(equip),
		 p(p), valor(valor), densEm(densEm), densMeus(densMeus),
		 densPunts(densPunts), densObst(densObst), nObstAdj(nObstAdj) {}

void operator=(const Objectiu& o) {
	equip = o.equip;
	p = o.p;
	valor = o.valor;
	densEm = o.densEm;
	densMeus = o.densMeus;
	densPunts = o.densPunts;
	densObst = o.densObst;
	nObstAdj = o.nObstAdj;
}
};

struct Control {
/* Clau = <id soldat/heli>, Valor = Posició */
unordered_map<int, queue<Pos> > MetesSd;
unordered_map<int, queue<Pos> > MetesHe;
unordered_map<int, Pos> DemandesSd;
unordered_map<int, Pos> DemandesHe;

struct InfoPos {
	int demSd;
	int metSd;
	int demHe;
	int metHe;
	int demPc;

	InfoPos() {}
	InfoPos(int demSd, int metSd, int demHe, int metHe, int demPc) 
	 		: demSd(demSd), metSd(metSd), demHe(demHe), metHe(metHe), demPc(demPc) {}
};

vector< vector<InfoPos> > N;

Control() {}

void inicialitzaControl() {
	N = vector< vector<InfoPos> >(MAX, vector<InfoPos>(MAX, InfoPos(0,0,0,0,0)));
}

inline bool estaDemanSd(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].demSd > 0;
}

inline bool estaDemanHe(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].demHe > 0;
}

inline int nDemanSd(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].demSd;
}

inline int nDemanHe(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].demHe;
}

inline bool estaDemanPc(const Pos& p) const {
	return N[p.x][p.y].demPc > 0;
}

inline int nDemanPc(const Pos& p) const {
	//assert(p.valida());
	return N[p.x][p.y].demPc;
}

inline bool esMetaSd(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].metSd > 0;
}

inline bool esMetaHe(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].metHe > 0;
}

inline int nMetesSd(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].metSd;
}

inline int nMetesHe(const Pos& p) const {
	//assert(p.valida());
    return N[p.x][p.y].metHe;
}

inline bool teMetaSd(int id) {
	auto it = MetesSd.find(id);
	return it != MetesSd.end() and not it->second.empty();
}

inline bool teMetaHe(int id) {
	auto it = MetesHe.find(id);
	return it != MetesHe.end() and not it->second.empty();
}

void demanaSd(const Pos& p, int id) {
	//assert(p.valida());
    ++N[p.x][p.y].demSd;
    DemandesSd[id] = p;
}

void demanaHe(const Pos& p, int id) {
	//assert(p.valida());
	++N[p.x][p.y].demHe;
    DemandesHe[id] = p;
}

void demanaPc(const Pos& p) {
	//assert(p.valida());
	++N[p.x][p.y].demPc;
}

void posaMetaSd(const Pos& p, int id) {
    //assert(p.valida());
    ++N[p.x][p.y].metSd;
    MetesSd[id].push(p);
}

void posaMetaHe(const Pos& p, int id) {
    //assert(p.valida());
    ++N[p.x][p.y].metHe;
    MetesHe[id].push(p);
}

void borraMetesSd(int id) {
	auto it = MetesSd.find(id);
	if (it != MetesSd.end()) {
   		while (not it->second.empty()) { 
   			--N[it->second.front().x][it->second.front().y].metSd;
   			it->second.pop();
   		}
   	}
}

Pos metaSd(int id) {
	if (teMetaSd(id)) return MetesSd[id].front();
	else return Pos(0, 0);
}

Pos metaHe(int id) {
	if (teMetaHe(id)) return MetesHe[id].front();
	else return Pos(0, 0);
}

void avancaMetaSd(int id) {
	if (teMetaSd(id)) {
		--N[MetesSd[id].front().x][MetesSd[id].front().y].metSd;
		MetesSd[id].pop();
	}
}

void avancaMetaHe(int id) {
	if (teMetaHe(id)) {
		--N[MetesHe[id].front().x][MetesHe[id].front().y].metHe;
		MetesHe[id].pop();
	}
}

void buidaDemandes() {
	DemandesSd.clear();
	DemandesHe.clear();
	for (vector<InfoPos>& v : N)
		for (InfoPos& i : v) i.demSd = i.demHe = i.demPc = 0;
}

void borraMetesMorts(const unordered_set<int>& m) {
   	for (auto it : MetesSd) {
   		if (not m.count(it.first)) {
   			while (not it.second.empty()) { 
   				--N[it.second.front().x][it.second.front().y].metSd;
   				it.second.pop();
   			}
   		}
   	}
}
};

struct IP {
	Pos p;
	int densEm;
	int densMeus;
	int densPunts;
	int densObst;
	int nObstAdj;

	IP() {}
	IP(const Pos& p, int densEm, int densMeus, int densPunts, int densObst, int nObstAdj) :
	p(p), densEm(densEm), densMeus(densMeus), densPunts(densPunts), densObst(densObst), nObstAdj(nObstAdj) {}
};

// Typedefs Generals
typedef vector< vector<int> > VVE;
typedef vector<bool> VB;
typedef vector<VB> VVB;
typedef list<int> LI;
// Typedefs Classes
typedef vector<Heli> VH;
typedef vector<Soldat> VS;
typedef vector<Objectiu> VO;
typedef unordered_map<int, Soldat> MES;
typedef unordered_map<int, Heli> MEH;
typedef vector<IP> VIP;
typedef unordered_map<int, unordered_set<int> > MSB;
// Moviments
typedef vector<Pos> VPO;
typedef vector<VP> VVPO;
typedef queue<Pos> QP;
typedef priority_queue<Pos> PQP;
typedef priority_queue<ResCami> PQRC;
typedef queue<ResCami> QRC;
// Constants
const Dir DRC[4] = { Dir(1, 0),Dir(0, 1),Dir(-1, 0),Dir(0, -1) };
const Dir DAC[8] = { Dir(0, 1),Dir(1, 0),Dir(-1, 0),Dir(0, -1), 
                     Dir(1, 1),Dir(-1, -1),Dir(1, -1),Dir(-1, 1) };

const Dir DSUD = Dir(1, 0);
const Dir DEST = Dir(0, 1);
const Dir DNORD = Dir(-1, 0);
const Dir DOEST = Dir(1, 0);
const Dir SENSE_DIR = Dir(0, 0);
struct PLAYER_NAME:public Player{  
static Player* factory(){return new PLAYER_NAME;}
VE _SA[5]; // Array de vectors de ids de soldats (Hi ha una pos. buida)
VE _HA[5]; // Array de vectors de ids de helis (Hi ha una pos. buida)

MES _S; // Mapa amb clau = id i valor = soldat amb id = id
MEH _H; // Mapa amb clau = id i valor = heli amb id = id

VO _OA[5]; // Array de vectors d'objectius on a la pos. 0 hi han els objectius
           // que no són de ningú, i a la resta de pos. 'i' hi han els objectius
           // de l'equip 'i'

int _Jefe;

Control _D; // Classe que emmagatzema les demandes de les posicions

VVB _I; // Indica si la posició és inútil (no accessible i sense posts)

VIP _PS; // Vector amb les posicions estudiades i la seva info

VVE _P; // Matriu de perill (en funció dels soldats enemics)

Dir _DA[8]; // Direccions dels soldats (es permutació de DA)
Dir _DR[4]; // Direccions dels helicòpters (es permutació de DRC)

int _estr; // Estratègia del jugador

int _paracas;

unordered_map<int, unordered_map<int, int> > _campejat;

unordered_map<int, unordered_map<int, int> > _HID;
// Estratègies
static const int TORN_A_MUERTE = 192;
static const int TORN_CONQUERIR = 140;
// Soldats
static const int MAX_SOLDATS_ATACAR = 60;
static const int MIN_SOLDATS_CONQUERIR = 25;
static const int MIN_SOLDATS_NAPALM = 4;
static const int MAX_DEMANDES_MATAR_SOLDAT_ADJ = 4;
static const int MIN_TORN_AUTONOM_SD = 14;
static const int MIN_TORN_JEFE_ATACA = 9;
static const int RANG_CAMPER_SD = 10;
// Càlcul densitats
static const int D_MAX = 13;
// Napalm
static const int TEMPS_ESCAPAR_NAPALM = 5;
void permutaDirs() {
    int i = 0; VE perm = permutacio(8);
    for (Dir& d : _DA) d = DAC[perm[i++]];
    i = 0; perm = permutacio(4);
    for (Dir& d : _DR) d = DRC[perm[i++]];
}

int nSoldatsMeus() {
	return _SA[JO].size();
}

int nSoldatsEm() {
	return NUM_SOLDATS*4 - nSoldatsMeus();
}

template <typename elem>
void insSort(vector<elem>&v,  bool (PLAYER_NAME::*vaAbans)(const elem&a, const elem&b, const Pos& p), const Pos& p) {
	int n = v.size();
	for (int i = 1; i < n; ++i) {
		elem x = v[i];
		int j;
		for (j = i; j > 0 and (this->*vaAbans)(x, v[j - 1], p); --j) v[j] = v[j - 1];
		v[j] = x;
	}
}
inline int terreny(const Pos& p) {
	//assert(p.valida());
    return que(p.x, p.y);
}

inline bool esGespa(const Pos& p) {
	//assert(p.valida());
    return terreny(p) == GESPA;
}

inline bool esBosc(const Pos& p) {
	//assert(p.valida());
    return terreny(p) == BOSC;
}

inline bool esMuntanya(const Pos& p) {
	//assert(p.valida());
    return terreny(p) == MUNTANYA;
}

inline bool esAigua(const Pos& p) {
	//assert(p.valida());
    return terreny(p) == AIGUA;
}

inline int tFoc(const Pos& p) {
	//assert(p.valida());
    return temps_foc(p.x, p.y);
}

inline bool hihaFoc(const Pos& p) {
	//assert(p.valida());
    return tFoc(p) > 0;
}

inline bool hihaFocAdjR(const Pos& p) {
	//assert(p.valida());
    for (const Dir& d : _DR) {
        if (p.valida() and not esMuntanya(p) and hihaFoc(p + d)) return true;
    }
    return false;
}

inline bool hihaHe(const Pos& p) {
	//assert(p.valida());
	return quin_heli(p.x, p.y) > 0;
}

inline int idHeR(const Pos& p) {
    auto it = _HID.find(p.x);
    if (it == _HID.end()) return 0;
    else {
        auto it2 = it->second.find(p.y);
        if (it2 == it->second.end()) return 0;
        else return it2->second;
    } 
}

inline bool hihaHeEm(const Pos& p) {
	//assert(p.valida());
	int id = quin_heli(p.x, p.y);
    return id > 0 and _H.at(id).equip != JO;
}

inline bool hihaHeMeu(const Pos& p) {
	//assert(p.valida());
	int id = quin_heli(p.x, p.y);
    return id > 0 and _H.at(id).equip == JO;
}

inline int deQuiObj(const Pos& p) {
    //assert(p.valida());
    return de_qui_post(p.x, p.y);
}

inline bool hihaObj(const Pos& p) {
	//assert(p.valida());
    return deQuiObj(p) != 0;
}

inline bool hihaObjNoMeta(const Pos& p) {
    //assert(p.valida());
    return deQuiObj(p) != 0 and not _D.esMetaSd(p);
}

inline bool hihaObjAlie(const Pos& p) {
	//assert(p.valida());
    int id = deQuiObj(p);
    return id != 0 and id != JO;
}

inline bool hihaObjAlieNoMeta(const Pos& p) {
    //assert(p.valida());
    int id = deQuiObj(p);
    return id != 0 and id != JO and not _D.esMetaSd(p);
}

inline bool hihaObjVAAlieNoMeta(const Pos& p) {
    //assert(p.valida());
    return hihaObjValuos(p) and hihaObjAlieNoMeta(p);
}

inline bool hihaObjVAAlie(const Pos& p) {
    //assert(p.valida());
    return hihaObjAlie(p) and hihaObjValuos(p);
}

inline bool hihaObjVANoMeta(const Pos& p) {
    return hihaObjValuos(p) and hihaObjNoMeta(p);
}

inline bool hihaObjEm(const Pos& p) {
	//assert(p.valida());
    int id = deQuiObj(p);
    return id > 0 and id != JO;
}

inline bool hihaObjLliure(const Pos& p) {
	//assert(p.valida());
    return deQuiObj(p) == -1;
}

inline bool hihaObjMeu(const Pos& p) {
	//assert(p.valida());
    return deQuiObj(p) == JO;
}

inline int valorObj(const Pos& p) {
	//assert(p.valida());
    //assert(hihaObj(p));
    return valor_post(p.x, p.y);
} 

inline bool hihaObjValuos(const Pos& p) {
    //assert(p.valida());
	return hihaObj(p) and valorObj(p) == VALOR_ALT;
}

inline bool hihaObjNoValuos(const Pos& p) {
    //assert(p.valida());
	return hihaObj(p) and valorObj(p) == VALOR_BAIX;
}

inline bool hihaSdEm(const Pos& p) {
	//assert(p.valida());
    int id = quin_soldat(p.x, p.y);
    return id > 0 and _S.at(id).equip != JO;
}

inline bool hihaSdMeu(const Pos& p) {
	//assert(p.valida());
    int id = quin_soldat(p.x, p.y);
    return id > 0 and _S.at(id).equip == JO;
}

inline bool hihaSd(const Pos& p) {
	//assert(p.valida());
	int id = quin_soldat(p.x, p.y);
	return id > 0;
}

int nSdEm5(const Pos&p) {
	//assert(p.valida());
    int nsoldats = 0;
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            if (p.valida() and hihaSdEm(Pos(i, j))) ++nsoldats;
        }
    }
    return nsoldats;
}

int nSdMeus5(const Pos&p) {
	//assert(p.valida());
    int nsoldats = 0;
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            if (p.valida() and hihaSdMeu(Pos(i, j))) ++nsoldats;
        }
    }
    return nsoldats;
}

int nPosFoc5(const Pos& p) {
	//assert(p.valida());
    int nposfoc = 0;
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            if (p.valida() and hihaFoc(Pos(i, j))) ++nposfoc;
        }
    }
    return nposfoc;
}

bool hihaObst(const Pos& p) {
    //assert(p.valida());
	return esMuntanya(p) or esAigua(p);
}

int idSd(const Pos& p) {
	//assert(p.valida() and hihaSd(p));
	return quin_soldat(p.x, p.y);
}

int idHe(const Pos& p) {
	//assert(p.valida() and hihaHe(p));
	return quin_heli(p.x, p.y);
}

int perill(const Pos& p) {
    return _P[p.x][p.y];
}

pair<bool, Pos> hihaHeEmDis(const Pos& p, int d) {
	for (int i = p.x - d; i <= p.x + d; ++i) {
		for (int j = p.y - d; j <= p.y + d; ++j) {
			Pos des(i, j);
			if (des.valida() and hihaHeEm(des)) return make_pair(true, des);
		}
	}
	return make_pair(false, Pos(0,0));
}

pair<bool, Pos> hihaHeEmNapDis(const Pos& p, int d) {
	for (int i = p.x - d; i <= p.x + d; ++i) {
		for (int j = p.y - d; j <= p.y + d; ++j) {
			Pos des(i, j);
			if (des.valida() and hihaHeEm(des) and 
                _H[idHe(des)].tNapalm <= TEMPS_ESCAPAR_NAPALM) 
                return make_pair(true, des);
		}
	}
	return make_pair(false, Pos(0,0));
}

pair<int, int> Obj(const Pos& p) {
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < int(_OA[i].size()); ++j) {
			if (_OA[i][j].p == p) return make_pair(i, j);
		}
	}
}

bool hihaObjCampejat(const Pos& p) {
	return hihaObj(p) and _campejat.find(p.x) != _campejat.end() and _campejat[p.x].find(p.y) != _campejat[p.x].end() 
		   and _campejat[p.x][p.y] != -1;
}

bool esMillorCampejar(const Pos&intent, const Pos& actual) {
    if (hihaObjMeu(intent) and hihaObjMeu(actual)) {
        if (valorObj(intent) == valorObj(actual)) return RANDOM;
        return valorObj(intent) > valorObj(actual);
    }
    return hihaObjMeu(intent);
}

bool hihaSdEmAdj(const Pos& p) {
    for (const Dir& d : _DA) {
        if (hihaSdEm(p + d)) return true;
    }
    return false;
}

pair<bool, Pos> hihaObjNoCampejatADis5(const Pos& p) {
    pair<bool, Pos> res;
    res.first = false;
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            if (Pos(i, j).valida() and hihaObj(Pos(i, j)) and not hihaObjCampejat(Pos(i, j))) {
                if (not res.first or esMillorCampejar(Pos(i, j), res.second)) {
                    res.first = true;
                    res.second = Pos(i, j);
                }
            }
        }
    }
    return res;
}

bool hihaObjVAAlieNoMetaHe(const Pos& p) {
    return hihaObjVAAlie(p) and not _D.esMetaHe(p);
}

bool hihaObjAlieNoMetaHe(const Pos& p) {
    return hihaObjAlie(p) and not _D.esMetaHe(p);
}

int nObstAdj(const Pos& p) {
    int cont = 0;
    for (const Dir&d : _DA) {
        Pos des = p + d;
        if (esAigua(des) or esMuntanya(des) or (hihaSdMeu(des) and _S[idSd(des)].camper and not _D.teMetaSd(idSd(des)))) ++cont;
    }
    return cont;
}
/* VALIDESA DE MOVIMENTS */
bool validaSdR(const Pos& p) {
    return p.valida() and (esGespa(p) or esBosc(p)) and not hihaFoc(p);
}

bool validaSdR(const Pos& p, int d) {
    return p.valida() and (esGespa(p) or esBosc(p)) and not hihaFoc(p); 
}

bool validaSdRND(const Pos& p, int d) {
    return p.valida() and (esGespa(p) or esBosc(p)) and not hihaFoc(p)
           and not _D.estaDemanSd(p); 
}

bool validaSd(const Pos& p) {
	return p.valida() and (esGespa(p) or esBosc(p)) and not hihaFoc(p)
		   and (not esBosc(p) or not hihaFocAdjR(p));
}

bool validaSdBFS(const Pos& p, int d) {
    return p.valida() and (esGespa(p) or esBosc(p)) and tFoc(p) < d
           and (not esBosc(p) or not hihaFocAdjR(p));
}

/* INFO DELS SOLDATS */
inline bool esMeu(const Soldat& s) {
    return s.equip == JO;
}

inline bool esEnem(const Soldat& s) {
    return s.equip != JO;
}

bool jaDirigit(const Soldat& s) {
	return s.dirigit;
}

bool esNouSd(const Soldat& s) {
	return s.nTorns == 0;
}

bool sonDelMateixEquip(const Soldat& s1, const Soldat& s2) {
	return s1.equip == s2.equip;
}

bool noPotFugir(const Soldat& s) {
    for (const Dir& d : _DA) {
        Pos des = s.p + d;
        if (validaSd(des) and not _D.estaDemanSd(des) and not hihaSdEm(des))
            return false;
    }
    return true;
}

bool hihaSdMeuAdj(const Pos& p) {
    for (const Dir& d : _DA) {
        if (hihaSdMeu(p + d)) return true;
    }
    return false;
}

bool hihaSdMeuNoASet(const Pos &p, unordered_map<int, unordered_set<int>> Set) {
   if (hihaSdMeu(p)) {
        auto it = Set.find(p.x);
        if (it == Set.end() or it->second.find(p.y) == it->second.end()) return true;
        return false;
    }
    return false;
}

int nXAdj(const Pos& p, bool (PLAYER_NAME::*potAnar)(const Pos&), unordered_map<int, unordered_set<int>>& Set) {
    int cont = 0;

    queue<Pos> S; S.push(p);
    while (not S.empty()) {
        Pos p = S.front(); S.pop();
        Set[p.x].insert(p.y);
        ++cont;
        for (const Dir& d : _DA) {
            Pos des = p + d;
            if ((Set.find(des.x) == Set.end() or Set[des.x].find(des.y) == Set[des.x].end()) and des.valida() and (this->*potAnar)(des)) {
                S.push(des);
            }
        }
    }

    return cont;
}

pair<bool, Pos> dinsAbastHeliEmNap(const Pos& p) {
    return hihaHeEmNapDis(p, A_H);
}

int nSdEq(int e) {
    //assert(e >= 1 and e <= 4);
    return _SA[e].size(); 
}

int costSd(const Pos& p) {
    //assert(p.valida());
    return _P[p.x][p.y] + hihaHeEm(p)*100;
}

bool senseConflicte(const Pos& p) {
    //assert(p.valida());
    return not hihaSdMeu(p);
}

/* OPERACIONS DE MANTENIMENT */
void netejaCampejatsMorts(const unordered_set<int>& nomort) {
    for (auto it : _campejat) {
        for (auto it2 : it.second) {
            it2.second = nomort.count(it2.second) ? it2.second : -1;
        } 
    }
}

void llegirSoldats() {
    for (int i = 1; i <= 4; ++i) _SA[i] = soldats(i);
    unordered_set<int> nomort;

    for (int i = 1; i <= 4; ++i) {
        for (int soldat : _SA[i]) {
            Info i = dades(soldat);
            if (i.equip == JO) nomort.insert(soldat);
            //assert(i != NUL);
            auto it = _S.find(i.id);
            if (it == _S.end()) {
                _S.insert(make_pair(soldat, Soldat(i.id, i.equip, i.vida, 0, 
                                    false, false, Pos(0,0), i.pos, SENSE_DIR, -1, -1, 0, 0, 0,
                                    false, false))); 
            }
            else {
                ++it->second.nTorns;
                it->second.d = director(it->second.p, Pos(i.pos));
                it->second.p = i.pos;
                it->second.vida = i.vida;
                it->second.dirigit = false;
            }
        }
    }

    _D.borraMetesMorts(nomort);
    netejaCampejatsMorts(nomort);
}

/* CRITERIS */
bool esMillorMatarIntentQueActual(int intent, int actual) {
    if (_D.nDemanSd(_S[intent].p) == _D.nDemanSd(_S[actual].p)) {
        if (esGespa(_S[intent].p) and esGespa(_S[actual].p)) {
            if (_S[intent].vida == _S[actual].vida) {
                if (nSdEq(_S[intent].equip) == nSdEq(_S[actual].equip)) return probabilitat(0.5);
                else return nSdEq(_S[intent].equip) > nSdEq(_S[actual].equip);
            }
            else return _S[intent].vida < _S[actual].vida;
        }
        else return esGespa(_S[intent].p);
    }
    else return _D.nDemanSd(_S[intent].p) > _D.nDemanSd(_S[actual].p);
}

bool esMillorJefeSd(int intent, int actual) {
    if (_S[intent].densEm == _S[actual].densEm) {
        if (_S[intent].densObst == _S[actual].densObst) {
            if (_S[intent].densMeus == _S[actual].densMeus) {
                if (_S[intent].densPunts == _S[actual].densPunts) return probabilitat(0.5);
                return _S[intent].densPunts > _S[actual].densPunts;
            }
            return _S[intent].densMeus > _S[actual].densMeus;
        }
        return _S[intent].densObst > _S[actual].densObst;
    }
    return _S[intent].densEm > _S[actual].densEm;
}

bool criteriObjectiu(const IP&a, const IP&b, const Pos& p) {
    if (hihaObj(a.p) and hihaObj(b.p)) {
        if (hihaObjVAAlie(a.p) and hihaObjVAAlie(b.p)) {
            if (hihaObjAlie(a.p) and hihaObjAlie(b.p)) {
                if (a.p.distancia(p) == b.p.distancia(p)) {
                    if (a.densPunts == b.densPunts) {
                        if (a.densEm == b.densEm) return RANDOM;
                        return a.densEm < b.densEm;
                    }
                    return a.densPunts > b.densPunts;
                }
                return a.p.distancia(p) < b.p.distancia(p);
            }
            return hihaObjAlie(a.p);
        }
        return hihaObjVAAlie(a.p);
    }
    return hihaObj(a.p);
}

bool criteriMassaEnem(const IP&a, const IP&b, const Pos& p) {
    if (hihaSdEm(a.p) and hihaSdEm(b.p)) {
        if (a.p.distancia(p) == b.p.distancia(p)) {
            if (a.densMeus == b.densMeus) {
                if (a.densPunts == b.densPunts) {
                    if (a.densObst == b.densObst) {
                        if (a.densMeus == b.densMeus) return a.densMeus < b.densMeus;
                    }
                    return a.densObst < b.densObst;
                }
                return a.densPunts > b.densPunts;
            }
            return a.densMeus > b.densMeus;
        }
        return a.p.distancia(p) < b.p.distancia(p);
    }
    return hihaSdEm(a.p);
}

bool criterisPosAtac(const IP&a, const IP&b, const Pos& p) {
    if (hihaSdEm(a.p) and hihaSdEm(b.p)) {
        if (a.densMeus == b.densMeus) return RANDOM;
        return a.densMeus > b.densMeus;
    }
    return hihaSdEm(a.p);
}

bool criterisDireccioSd(const Soldat& s, const Dir& intent, const Dir& actual, const Dir& i, const Pos& f) {
    if (not hihaSdMeu(s.p + intent) and not hihaSdMeu(s.p + actual)) {
        if (_D.nDemanSd(Pos(s.p + intent)) == _D.nDemanSd(Pos(s.p + actual))) {
            if (hihaSdEmAdj(s.p + intent) and hihaSdEmAdj(s.p + actual)) {
                if (intent.angle(i) == actual.angle(i))  {
                    if (intent.angle(director(s.p, f)) == actual.angle(director(s.p, f)))
                        return probabilitat(0.5);
                    else return intent.angle(director(s.p, f)) < actual.angle(director(s.p, f));
                }
                else return intent.angle(i) < actual.angle(i);
            }
            else return hihaSdEm(s.p + intent);
        }
        else return _D.nDemanSd(Pos(s.p + intent)) < _D.nDemanSd(Pos(s.p + actual));
    }
    else return not hihaSdMeu(s.p + intent);
}

/* TRIA DE POSICIONS I SOLDATS */
Pos escullPosAtac(const Soldat& s) {
    insSort(_PS, &PLAYER_NAME::criterisPosAtac, s.p);
    return _PS[0].p;
}

Pos escullObjectiu(const Soldat& s) {
    Pos res;
    insSort(_PS, &PLAYER_NAME::criteriObjectiu, s.p);
    res = _PS[0].p;
    return res;
}

Pos escullMassaEnemics(const Soldat& s) {
    Pos res;
    insSort(_PS, &PLAYER_NAME::criteriMassaEnem, s.p);
    res = _PS[0].p;
    return res;
}

void escullJefeSd() {
    bool jaAssignat = false;
    int actual = _SA[JO][0];
    for (int s : _SA[JO]) {
        int intent = s;
        if (not jaAssignat or esMillorJefeSd(intent, actual)) {
            actual = intent;
            jaAssignat = true;
        } 
    }
    //assert(jaAssignat);
    _Jefe = actual;
}

/* ORDENACIÓ DE MOVIMENTS */
void ordenaSoldat(Soldat& s, Pos p) {
	if (not validaSdR(p)) p.escriu();
	//assert(validaSdR(p));

    _D.demanaSd(p, s.id);
    ordena_soldat(s.id, p.x, p.y);

    //assert(not s.dirigit);
    s.dirigit = true;
}

void noEtMoguisSd(Soldat& s) {
    _D.demanaSd(s.p, s.id);
    //assert(not s.dirigit);
    s.dirigit = true;
}

void vesSd(Soldat& s, const ResCami& r, bool meta) {
    if (not r.trobat or r.d == SENSE_DIR) noEtMoguisSd(s);
    else if (senseConflicte(s.p + r.d)) {
        ordenaSoldat(s, s.p + r.d);
        if (meta) _D.posaMetaSd(r.p, s.id);
    }
    else executaAlternativesSd(s, r.d, r.p, meta);
}

void mataSoldatAdj(Soldat&s) {
    bool trobat = false;
    int actual;
    for (const Dir& d : _DA) {
        Pos des = s.p + d;
        if (validaSd(des) and hihaSdEm(des) and 
            _D.nDemanSd(des) <= MAX_DEMANDES_MATAR_SOLDAT_ADJ) {
            int intent = idSd(des);
            if (not trobat or esMillorMatarIntentQueActual(intent, actual)) {
                actual = intent;
                trobat = true;
            }
        } 
    }

    if (trobat) ordenaSoldat(s, _S[actual].p);
}

void conquereixObjAdj(Soldat& s) {
    for (const Dir& d : _DA) {
        Pos des = s.p + d;
        if (validaSd(des) and hihaObjAlie(des) and not _D.estaDemanSd(des)) ordenaSoldat(s, des);
    }
}

/* TORN A MORT */
void inicialitzaTornAMort(Soldat& s) {
    _D.borraMetesSd(s.id);
    ResCami res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjVAAlieNoMeta, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjVAAlie, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjVANoMeta, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjAlieNoMeta, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjAlie, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjNoMeta, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaSdEm, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObjValuos, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, &PLAYER_NAME::validaSdR, &PLAYER_NAME::hihaObj, &PLAYER_NAME::costSd, TORNS);
    vesSd(s, res, true);
}

void avancaTornAMort(Soldat& s) {
    ResCami res = WSPSd(s.p, _D.MetesSd.at(s.id).front(), &PLAYER_NAME::validaSdRND, &PLAYER_NAME::costSd, TORNS);
    if (not res.trobat) res = WSPSd(s.p, _D.MetesSd.at(s.id).front(), &PLAYER_NAME::validaSdR, &PLAYER_NAME::costSd, TORNS);
    vesSd(s, res, false);
}

void gestionaTornAMort(Soldat& s) {
    if (TORN == TORN_A_MUERTE) inicialitzaTornAMort(s);
    else if (_D.teMetaSd(s.id)) avancaTornAMort(s);
    else {
    	executaEstrConqSd(s);
        if (not jaDirigit(s)) noEtMoguisSd(s);
    }
}

/* CAMPEJAMENT */
bool buscaPosPocPerill(const Pos& p, Pos& res) {
	bool trobat = false;
	for (int i = p.x - min(RANG_CAMPER_SD, TORNS - 2); i <= p.x + min(RANG_CAMPER_SD, TORNS - 2); ++i) {
		for (int j = p.y - min(RANG_CAMPER_SD, TORNS - 2); j <= p.y + min(RANG_CAMPER_SD, TORNS - 2); ++j) {
			if (validaSd(Pos(i, j)) and nObstAdj(Pos(i, j)) <= 3 and (not trobat or _P[i][j] < _P[res.x][res.y])) {
				res = Pos(i, j);
				trobat = true;
			}
		}
	}
	return trobat;
}

void campejaObjSd(Soldat& s, const Pos& p) {
	Pos des;
    bool trobat = buscaPosPocPerill(p, des);
   	if (trobat) {
    	_D.borraMetesSd(s.id);
    	_D.posaMetaSd(des, s.id);
    	_campejat[p.x][p.y] = s.id;
        s.camper = true;
        s.camp = p;
    }
}

/* EXECUCIÓ D'ESTRATÈGIES */
void agrupaSoldats(Soldat& s) {
    //assert(_Jefe > 0);
    if (_Jefe == s.id) {
        if (TORN < MIN_TORN_JEFE_ATACA) noEtMoguisSd(s);
        else {
            ResCami res = BFSSd(s.p, &PLAYER_NAME::validaSdBFS, &PLAYER_NAME::hihaSdEm);
            vesSd(s, res, false);
        }
    }
    else {
        ResCami res = BFSSd(s.p, _S[_Jefe].p, &PLAYER_NAME::validaSdBFS);
        if (res.trobat) vesSd(s, res, false);
        else {
            res = BFSSd(s.p, &PLAYER_NAME::validaSdBFS, &PLAYER_NAME::hihaSdEm);
            vesSd(s, res, false);
        }
    }
}

void executaEstrConqSd(Soldat& s) {
    pair<bool, Pos> campeo = hihaObjNoCampejatADis5(s.p);
    if (campeo.first) campejaObjSd(s, campeo.second);
    bool teMeta = _D.teMetaSd(s.id);

    while (teMeta and s.p == _D.metaSd(s.id)) {
    	_D.avancaMetaSd(s.id);
    	teMeta = _D.teMetaSd(s.id);
    }

    if (teMeta) {
    	ResCami des = BFSSd(s.p, _D.metaSd(s.id), &PLAYER_NAME::validaSdBFS);
    	vesSd(s, des, false);
    }
    else {
        if (s.camper) {
            ResCami des = BFSSd(s.p, s.camp, &PLAYER_NAME::validaSdBFS);
            if (des.dist >= TORNS - 2) vesSd(s, des, true);
            else noEtMoguisSd(s);
        }
        else {
        	ResCami des = BFSSd(s.p, escullObjectiu(s), &PLAYER_NAME::validaSdBFS);
        	vesSd(s, des, false);
        }
    }
}

bool hihaSdMeuAmbMesDensMeus(const Pos&p, int id) {
	return hihaSdMeu(p) and _S[idSd(p)].densMeus > _S[id].densMeus;
}

void executaEstrAtacSd(Soldat& s) {
    ResCami des;
    /*unordered_map<int, unordered_set<int>> Set;
    if (nXAdj(s.p, &PLAYER_NAME::hihaSdMeu, Set) <= 2) {
        QRC Q;
        VVB S(MAX, VB(MAX, false)); S[s.p.x][s.p.y] = true;

        for (const Dir& d : _DA) { 
            Pos n = s.p + d; S[n.x][n.y] = true;
            if (validaSdBFS(n, 1)) Q.push(ResCami(d, n, 1, _P[n.x][n.y]));
        }

        while (not Q.empty() and not hihaSdMeuNoASet(Q.front().p, Set)) {
            ResCami a = Q.front(); Q.pop();
            for (const Dir& d : _DA) { 
                Pos n = a.p + d;
                if (not S[n.x][n.y] and validaSdBFS(n, a.dist + 1)) {
                    S[n.x][n.y] = true;
                    Q.push(ResCami(a.d, n, a.dist+1, a.cost+_P[n.x][n.y]));
        }   }   }

        if (not Q.empty()) des = ResCami (Q.front().d, Q.front().p, 
                                          Q.front().dist, Q.front().cost, true);
        else des = BFSSd(s.p, &PLAYER_NAME::validaSdBFS, &PLAYER_NAME::hihaSdEm);
    }*/
    des = BFSSd(s.p, &PLAYER_NAME::validaSdBFS, &PLAYER_NAME::hihaSdEm);
    ResCami aux = des;
    if (des.trobat and _S[idSd(des.p)].densEm > s.densMeus) {
    	des = WSPSd(s.p, &PLAYER_NAME::validaSdBFS, s.id, &PLAYER_NAME::hihaSdMeuAmbMesDensMeus, &PLAYER_NAME::costSd);
    	if (not des.trobat) vesSd(s, aux, false);
    	else vesSd(s, des, false);
    }
    else vesSd(s, des, false);
}

void executaAlternativesSd(Soldat& s, const Dir& i, const Pos& f, bool meta) {
    Dir act;
    bool trobat = false;
    for (const Dir& d : _DA) {
        if (validaSd(s.p + d) and (not trobat or criterisDireccioSd(s, d, act, i, f))) {
            act = d;
            trobat = true;
        }
    }

    if (trobat) {
        ordenaSoldat(s, s.p + act);
        if (meta) _D.posaMetaSd(f, s.id);
    }
    else noEtMoguisSd(s); 
}

bool hihaSdEmADisd(const Pos& p, int dis) {
    for (int i = p.x - dis; i <= p.x + dis; ++i) {
        for (int j = p.y - dis; j <= p.y + dis; ++j) {
            Pos des = Pos(i, j);
            if (des.valida() and hihaSdEm(des) and p.distancia(des) == 2) return true;
        }
    }
    return false;
}

void executaImmediatsSd(Soldat& s) {
    if (TORNS > 1) mataSoldatAdj(s);
    if (not jaDirigit(s)) conquereixObjAdj(s);
}

/* DIRECCIÓ GENERAL */
void dirigeixSoldat(Soldat&s) {
	if (jaDirigit(s)) return;
    
    // Gestió torns finals
        	executaImmediatsSd(s);
        	if (jaDirigit(s)) return;
    if (_estr == A_MUERTE and not s.camper) gestionaTornAMort(s);
    else {
    	if (not jaDirigit(s) and TORN <= MIN_TORN_AUTONOM_SD) agrupaSoldats(s);
    	else if (not jaDirigit(s)) {
        	switch(_estr) {
            	case ATACAR: executaEstrAtacSd(s); break;
            	case FER_NAPALM:
                case A_MUERTE:
            	case CONQUERIR: executaEstrConqSd(s); break;
            	default:; //assert(false);
}   }	}   }

void dirigeixSoldats() {
    if (TORN == 0) escullJefeSd();
    
    for (int s : _SA[JO]) {
        //assert(_S.count(s));
        dirigeixSoldat(_S[s]); 
	}
}
bool validaHe(const Pos& p, int id) {
    int idE;
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            Pos des = Pos(i, j);
            if (not des.valida() or esMuntanya(des) or ((idE = idHeR(des)) > 0 and idE != id)) return false;
        }
    }
    return true;
}

bool validaHeRel(const Pos& p, int id) {
    for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
        for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
            Pos des = Pos(i, j);
            if (not des.valida() or esMuntanya(des)) return false;
        }
    }
    return true;
}

bool validaPc(const Pos& p) {
	return validaSd(p) and not hihaSdMeu(p) and not hihaSdEm(p)
           and not _I[p.x][p.y] and not _D.estaDemanSd(p);
}

bool jaDirigit(const Heli&h) {
	return h.dirigit;
}

bool sonDelMateixEquip(const Heli& h1, const Heli& h2) {
	return h1.equip == h2.equip;
}

void llegirHelis() {
    _HID = unordered_map<int, unordered_map<int, int> >();

    for (int i = 1; i <= 4; ++i) _HA[i] = helis(i);
    for (int i = 1; i <= 4; ++i) {
        for (int heli : _HA[i]) {
            Info inf = dades(heli);
            //assert(inf != NUL);
            if (TORN == 0) {
                _H.insert(make_pair(inf.id, Heli(inf.id, inf.equip, inf.orientacio, 
                                    inf.napalm, false, inf.pos, Pos(0,0), SENSE_DIR,
                                    inf.paraca)));
            }
            else {
                auto it = _H.find(inf.id);
            	it->second.d = director(it->second.p, Pos(inf.pos));
                it->second.p = inf.pos;
                it->second.orie = inf.orientacio;
                it->second.tNapalm = inf.napalm;
                it->second.paracas = inf.paraca;
                it->second.dirigit = false;
            }

            for (int i = inf.pos.x - ABAST; i <= inf.pos.x + ABAST; ++i) {
                for (int j = inf.pos.y - ABAST; j <= inf.pos.y + ABAST; ++j) {
                    _HID[i][j] = inf.id;
                }
            }
        }
    }
}

void ordenaHeli(Heli&h, const Pos& n, const int ordre) {
	//assert(validaHe(n, h.id));
    _D.demanaHe(n, h.id);
    ordena_helicopter(h.id, ordre);
    //assert(not h.dirigit);
    h.dirigit = true;
}

void noFacisReHe(Heli& h) {
	_D.demanaHe(h.p, h.id);
	//assert(not h.dirigit);
	h.dirigit = true;
}

void ordenaPc(Pos& des) {
	//assert(validaPc(des));
	_D.demanaPc(des);
	ordena_paracaigudista(des.x, des.y);
}

int calculaGir(const Heli&h, const Dir&d) {
	//assert(d.validaHe());
    int dirRel = Dir(h.orie).direccioRel(d);
    if (dirRel == OPOSADES) return probabilitat(0.5) ? RELLOTGE : CONTRA_RELLOTGE;
    else if (dirRel == EQUIVALENTS) return NO_GIRIS;
    else return dirRel;
}

bool notInSet(const unordered_map<int, unordered_set<int>>&set, const Pos& p) {
	auto it = set.find(p.x);
	if (it != set.end()) {
		auto it2 = it->second.find(p.y);
		return it2 == it->second.end();
	}
	else return true;
}

bool criterisParaca(const Pos& actual, const Pos& intent, const unordered_map<int, unordered_set<int>>& jaEsc) {
	if (notInSet(jaEsc, actual) and notInSet(jaEsc, intent)) {
		if (not _D.estaDemanSd(actual) and not _D.estaDemanSd(intent)) {
			if (hihaObj(actual) and hihaObj(intent)) {
				if (hihaSdEmAdj(actual) and hihaSdEmAdj(intent)) {
					if (esBosc(actual) and esBosc(intent)) return RANDOM;
					else return esBosc(intent);
				}
				else return hihaSdEmAdj(intent);
			}
			else return hihaObj(actual);
		}
		else return not _D.estaDemanSd(intent);
	}
	else return notInSet(jaEsc, intent);
}

Pos posPc(const Pos& p, const unordered_map<int, unordered_set<int>>& jaEsc) {
	Pos escollida = Pos(0,0);
	
	bool trobat = false;
	for (int i = p.x - ABAST; i <= p.x + ABAST; ++i) {
		for (int j = p.y - ABAST; j <= p.y + ABAST; ++j) {
			Pos des = Pos(i, j);
			if (validaPc(des) and (not trobat or criterisParaca(escollida, des, jaEsc))) {
				escollida = des;
				trobat = true;
			}
		}
	}

	return escollida;
}

void dirigeixParacas(Heli&h) {
	unordered_map<int, unordered_set<int>> jaEsc;
    int i = h.p.x - ABAST;
    int paraca = 0;
    if (TORNS < 15 or h.paracas.size() >= 4 or (h.paracas.size() > 0 and h.paracas[0] <= 2)) {
    	while (_paracas < 4 and paraca < int(h.paracas.size())) {
    		Pos des = posPc(h.p, jaEsc);
    		if (des != Pos(0,0) and notInSet(jaEsc, des)) {
    			ordenaPc(des);
    			jaEsc[des.x].insert(des.y);
    		}
    		++paraca; ++_paracas;
    	}
    }
}


bool criterisPosAtacHe(const IP&a, const IP& b, const Pos& p) {
    if (a.densMeus == b.densMeus) {
        if (a.densEm == b.densEm) return RANDOM;
        return a.densEm > b.densEm;
    }
    return a.densMeus > b.densMeus;
}   

Pos escullPosAtacHe(const Heli& h) {
    if (TORN%30 == 0) {
        int maxsoldats = 0;
        int equip = 0;
        for (int i = 1; i <= 4; ++i) {
            if (i != JO and nSdEq(i) > maxsoldats) {
                maxsoldats = nSdEq(i);
                equip = i;
            }
        }

        Pos p = Pos(30,30);
        if (_SA[equip].size() > 0) {
            p = _S[_SA[equip][uniforme(0, _SA[equip].size() - 1)]].p;
        }
        /*
        int maxit = 200;
        int i = 0;
        Pos random;
        random.x = uniforme(3, 57);
        random.y = uniforme(3, 57);
        while (i < maxit and (not hihaSdEm(random) or not validaHe(random, h.id))) {
            random.x = uniforme(3, 57);
            random.y = uniforme(3, 57);
            ++i;
        }*/
        return p;
    }
    else return h.desti;
}

void executaEstrAtacHe(Heli& h) {
    //h.desti = escullPosAtacHe(h);
    ResCami res;
    if (TORN <= 70) res = WSPHe(h.p, h.id, Dir(h.orie), &PLAYER_NAME::validaHe, &PLAYER_NAME::hihaSdMeu);
   	else res = WSPHe(h.p, h.id, Dir(h.orie), &PLAYER_NAME::validaHe, &PLAYER_NAME::hihaSdEm);
    if (res.trobat and res.d != SENSE_DIR) {
        int gir = calculaGir(h, res.d);
        if (gir == NO_GIRIS) ordenaHeli(h, h.p + res.d + res.d2, res.d == res.d2 ? AVANCA2 : AVANCA1);
        else ordenaHeli(h, h.p, gir);
    }
}

void vesHe(Heli&h, const ResCami& r, bool meta) {
    if (r.trobat and r.d != SENSE_DIR) {
        int gir = calculaGir(h, r.d);
        if (gir == NO_GIRIS) ordenaHeli(h, h.p + r.d + r.d2, r.d == r.d2 ? AVANCA2 : AVANCA1);
        else ordenaHeli(h, h.p, gir);
        if (meta) _D.posaMetaHe(r.p, h.id);
    }
}

void executaEstrConqHe(Heli& h) {
    bool teMeta = _D.teMetaHe(h.id);

    while (teMeta and h.p.distancia(_D.metaHe(h.id)) <= 2) {
        _D.avancaMetaHe(h.id);
        teMeta = _D.teMetaHe(h.id);
    }

    if (teMeta) {
        ResCami res = WSPHe(h.p, _D.metaHe(h.id), h.id, Dir(h.orie), &PLAYER_NAME::validaHe);
        vesHe(h, res, false);
    }
    else {
        ResCami res = WSPHe(h.p, h.id, Dir(h.orie),&PLAYER_NAME::validaHe, &PLAYER_NAME::hihaObjVAAlieNoMetaHe);
        vesHe(h, res, true);
    }
}

bool criterisNapalm(const IP&a, const IP& b, const Pos& p) {
    if (a.densEm == b.densEm) {
        if (a.p.distancia(p) == b.p.distancia(p)) return RANDOM;
        return a.p.distancia(p) < b.p.distancia(p);
    } 
    return a.densEm > b.densEm;
}

void executaEstrNapalm(Heli& h) {
    insSort(_PS, &PLAYER_NAME::criterisNapalm, h.p);
    ResCami res = WSPHe(h.p, _PS[0].p, h.id, Dir(h.orie), &PLAYER_NAME::validaHe);
    if (h.p.distancia(res.p) <= 1 and h.tNapalm == 0) ordenaHeli(h, h.p, NAPALM);
    else vesHe(h, res, false);
}

void dirigeixHeli(Heli&h) {
	if (jaDirigit(h)) return;

    if (_estr != FER_NAPALM and h.tNapalm == 0 and nSdEm5(h.p) >= 4 and nSdMeus5(h.p) <= 2) ordenaHeli(h, h.p, NAPALM);
    if (jaDirigit(h)) return;
    switch (_estr) {
        case ATACAR: executaEstrAtacHe(h);
        break;
        case A_MUERTE: case CONQUERIR:
        executaEstrConqHe(h);
        break;
        case FER_NAPALM:
        executaEstrNapalm(h);
        break;
    	default:; //assert(false);
    }

    dirigeixParacas(h);
}

void dirigeixHelis()  {
    _paracas = 0;
    int heliIni = _H[_HA[JO][0]].paracas.size() > _H[_HA[JO][1]].paracas.size() ? 0 : 1;
    dirigeixHeli(_H[_HA[JO][heliIni]]); dirigeixHeli(_H[_HA[JO][heliIni ? 0 : 1]]); 
}
inline bool esMeu(const Objectiu& o) {
    return o.equip == JO;
}

inline bool esDeNingu(const Objectiu& o) {
    return o.equip == DE_NINGU;
}

inline bool esDeEm(const Objectiu& o) {
    return not esMeu(o) and not esDeNingu(o); 
}

inline bool esValuosObj(const Objectiu& o) {
	return o.valor == VALOR_ALT;
}

void llegirObjs() {
    VP2 vpost = posts();
    for (VO& v : _OA) v = VO();
    for (const Post& pt : vpost) {
        Objectiu obj = Objectiu(pt.equip, pt.pos, pt.valor, 0,0,0,0,0);
        if (esDeNingu(obj)) _OA[0].push_back(obj);
        else _OA[obj.equip].push_back(obj);
}   }
static const int ATACAR = 1;
static const int CONQUERIR = 2;
static const int A_MUERTE = 3;
static const int FER_NAPALM = 4;

void definirEstrategia() {
	if (TORN == 0) _estr = ATACAR;
	else if (TORN >= TORN_A_MUERTE) _estr = A_MUERTE;
	else if (nSoldatsMeus() < MIN_SOLDATS_NAPALM) _estr = FER_NAPALM;
	else if (TORN >= TORN_CONQUERIR or nSoldatsMeus() > MAX_SOLDATS_ATACAR or
		     (_estr == CONQUERIR and nSoldatsMeus() > MIN_SOLDATS_CONQUERIR))
				_estr = CONQUERIR;
	else _estr = ATACAR;
}
void guardaInfo(int a1, int a2, int d, Soldat& s) {
	Pos act(a1, a2);
	if (not esMuntanya(act) and not esAigua(act) and not _I[act.x][act.y]) {
		if (s.equip != JO) _P[act.x][act.y] += esBosc(act) ? 2*(D_MAX - d) : D_MAX - d;
		if (hihaSdEm(act)) s.densEm += D_MAX - d;
		if (hihaSdMeu(act)) s.densMeus += D_MAX - d;
		if (hihaObj(act))  s.densPunts += (D_MAX - d)*(valorObj(act)/1000);
	}
	else {
		s.densObst += D_MAX - d;
		if (d == 1) ++s.nObstAdj;
	}
}


void guardaInfo(int a1, int a2, int d, Objectiu& o) {
	Pos act(a1, a2);
	if (not esMuntanya(act) and not esAigua(act) and not _I[act.x][act.y]) {
		if (hihaSdEm(act)) o.densEm += D_MAX - d;
		if (hihaSdMeu(act)) o.densMeus += D_MAX - d;
		if (hihaObj(act))  o.densPunts += (D_MAX - d)*(valorObj(act)/1000);
	}
	else {
		o.densObst += D_MAX - d;
		if (d == 1) ++o.nObstAdj;
	}
}

void analitzaDis(int i, int j, int d, VVE& V, Soldat& s, Objectiu& o, bool sd) {
    if (i - d > 0) {
        int ini = max(j - d, 1); int fi = min(j + d, MAX - 2);
        for (int k = ini; k <= fi; ++k) {
        	if (not V[i - d][k]) {
        		V[i - d][k] = true;
        		if (sd) guardaInfo(i - d, k, d, s);
        		else guardaInfo(i - d, k, d, o);
    }	}	}

    if (i + d < MAX - 1) {
        int ini = max(j - d, 1); int fi = min(j + d, MAX - 2);
        for (int k = ini; k <= fi; ++k) {
        	if (not V[i + d][k]) {
        		V[i + d][k] = true;
        		if (sd) guardaInfo(i + d, k, d, s);
        		else guardaInfo(i + d, k, d, o);
    }	}	}

    if (j - d > 0) {
        int ini = max(i - d, 1); int fi = min(i + d, MAX - 2);
        for (int k = ini; k <= fi; ++k) {
        	if (not V[k][j - d]) {
        		V[k][j - d] = true;
        		if (sd) guardaInfo(k, j - d, d, s);
        		else guardaInfo(k, j - d, d, o);
    }	}	}

    if (j + d < MAX - 1) {
        int ini = max(i - d, 1); int fi = min(i + d, MAX - 2);
        for (int k = ini; k <= fi; ++k) {
        	if (not V[k][j + d]) {
        		V[k][j + d] = true;
        		if (sd) guardaInfo(k, j + d, d, s);
        		else guardaInfo(k, j + d, d, o);
}	}	}	}

void analitzaCasella(int i, int j, Soldat& s) {
	VVE V(MAX, VE(MAX, false));
	Objectiu o = Objectiu();
    for (int d = 0; d <= D_MAX; ++d) analitzaDis(i, j, d, V, s, o, true);
    _PS.push_back(IP(Pos(i, j), s.densEm, s.densMeus, s.densPunts, s.densObst, s.nObstAdj));
}

void analitzaCasella(int i, int j, Objectiu& o) {
	VVE V(MAX, VE(MAX, false));
	Soldat s = Soldat();
    for (int d = 0; d <= D_MAX; ++d) analitzaDis(i, j, d, V, s, o, false);
    _PS.push_back(IP(Pos(i, j), o.densEm, o.densMeus, o.densPunts, o.densObst, o.nObstAdj));
}

void calculaDensitats() {
	int maxDensEm = 0; int maxDensMeus = 0;
	int maxDensPunts = 0; int minDensEmObj = INFINIT;
	int minDensEmObjH = INFINIT;
	for (const VE& v : _SA) {
		for (int s : v) {
			int i = _S[s].p.x; int j = _S[s].p.y;

			int prevdensEm = _S[s].densEm != -1 ? _S[s].densEm : INFINIT;
			int prevdensMeus = _S[s].densMeus != -1 ? _S[s].densMeus : INFINIT;
			_S[s].densEm = _S[s].densMeus = _S[s].densObst = _S[s].nObstAdj =
            _S[s].densPunts = 0;

			analitzaCasella(i, j, _S[s]);

			_S[s].incDensEm = _S[s].densEm > prevdensEm;
			_S[s].incDensMeus = _S[s].densMeus > prevdensMeus;			
	}	}

	for (VO& v : _OA)
		for (Objectiu& o : v) analitzaCasella(o.p.x, o.p.y, o);
}

void marca_BFS(const Pos& p) {
    QP Q; Q.push(p); _I[p.x][p.y] = false;

    while (not Q.empty()) {
        Pos a = Q.front(); Q.pop();
        for (const Dir& d : _DA) {
        	Pos n = a + d;
            if (_I[n.x][n.y] and (esGespa(n) or esBosc(n))) {
            	Q.push(n);
            	_I[n.x][n.y] = false;
}	}   }	}

void marcaInutils() {
	_I = VVB(MAX, VB(MAX, true));
	for (const VO& v : _OA) {
		for (const Objectiu& o : v) marca_BFS(o.p);
}	}

void analitzaMapa() {
	if (TORN == 0) marcaInutils();
	_P = VVE(MAX, VE(MAX, 0));
	_PS = VIP();
	calculaDensitats();
}
ResCami BFSSd(const Pos&i, const Pos& f, 
               bool (PLAYER_NAME::*potAnar)(const Pos&, int d),
               int maxd = INFINIT) 
{
    if (i == f) return ResCami(SENSE_DIR, i, 0, 0, true);

    QRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;

    for (const Dir& d : _DA) { 
        Pos n = i + d; S[n.x][n.y] = true;
        if ((this->*potAnar)(n, 1)) Q.push(ResCami(d, n, 1, _P[n.x][n.y]));
    }

    while (not Q.empty() and Q.front().p != f) {
        ResCami a = Q.front(); Q.pop();
        for (const Dir& d : _DA) { 
            Pos n = a.p + d;
            if (not S[n.x][n.y] and (this->*potAnar)(n, a.dist + 1)) {
                S[n.x][n.y] = true;
                if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist+1, a.cost+_P[n.x][n.y]));
    }   }   }

    if (not Q.empty()) return ResCami(Q.front().d, Q.front().p, 
                                      Q.front().dist, Q.front().cost, true);
    else return ResCami(false);
}

ResCami BFSSd(const Pos&i, bool (PLAYER_NAME::*potAnar)(const Pos&, int d),
              bool (PLAYER_NAME::*final)(const Pos&), int maxd = INFINIT, bool potPrimera = true) 
{
    if ((this->*final)(i) and potPrimera) return ResCami(SENSE_DIR, i, 0, 0, true); 

    QRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;

    for (const Dir& d : _DA) { 
        Pos n = i + d; S[n.x][n.y] = true;
        if ((this->*potAnar)(n, 1)) Q.push(ResCami(d, n, 1, 1));
    }

    while (not Q.empty() and not (this->*final)(Q.front().p)) {
        ResCami a = Q.front(); Q.pop();
        for (const Dir& d : _DA) { 
            Pos n = a.p + d;
            if (not S[n.x][n.y] and (this->*potAnar)(n, a.dist + 1)) {
                S[n.x][n.y] = true;
                if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist+1, a.cost+1));
    }   }   }

    if (not Q.empty()) return ResCami(Q.front().d, Q.front().p, 
                                      Q.front().dist, Q.front().cost, true);
    else return ResCami(false);
}

ResCami BFSSd(const Pos&i, bool (PLAYER_NAME::*potAnar)(const Pos&, int d), int id,
              bool (PLAYER_NAME::*final)(const Pos&, int id), int maxd = INFINIT, bool potPrimera = true) 
{
    if ((this->*final)(i, id) and potPrimera) return ResCami(SENSE_DIR, i, 0, 0, true); 

    QRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;

    for (const Dir& d : _DA) { 
        Pos n = i + d; S[n.x][n.y] = true;
        if ((this->*potAnar)(n, 1)) Q.push(ResCami(d, n, 1, 1));
    }

    while (not Q.empty() and not (this->*final)(Q.front().p, id)) {
        ResCami a = Q.front(); Q.pop();
        for (const Dir& d : _DA) { 
            Pos n = a.p + d;
            if (not S[n.x][n.y] and (this->*potAnar)(n, a.dist + 1)) {
                S[n.x][n.y] = true;
                if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist+1, a.cost+1));
    }   }   }

    if (not Q.empty()) return ResCami(Q.front().d, Q.front().p, 
                                      Q.front().dist, Q.front().cost, true);
    else return ResCami(false);
}

ResCami WSPSd(const Pos&i, bool (PLAYER_NAME::*potAnar)(const Pos&, int d), int id, 
              bool (PLAYER_NAME::*final)(const Pos&, int id), int (PLAYER_NAME::*cost)(const Pos&), int maxd = INFINIT) 
{
    if ((this->*final)(i, id)) return ResCami(SENSE_DIR, i, 0, 0, true);
    PQRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;
    VVE C(MAX, VE(MAX, INFINIT));

    for (const Dir& d : _DA) { 
        Pos n = i + d;
        if ((this->*potAnar)(n, 1)) { 
            C[n.x][n.y] = (this->*cost)(n) + 100000; 
            Q.push(ResCami(d, n, 1, C[n.x][n.y])); 
    }   }

    while (not Q.empty() and not (this->*final)(Q.top().p, id)) {
        ResCami a = Q.top(); Q.pop();
        if (not S[a.p.x][a.p.y]) {
            S[a.p.x][a.p.y] = true;
            for (const Dir& d : _DA) { 
                Pos n = a.p + d;
                int c = (this->*cost)(n) + 100000;
                if ((this->*potAnar)(n, a.dist + 1) and 
                    C[a.p.x][a.p.y] + c < C[n.x][n.y]) {
                    C[n.x][n.y] = C[a.p.x][a.p.y] + c; 
                    if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist + 1, C[n.x][n.y]));
    }   }   }   }

    if (not Q.empty()) 
        return ResCami(Q.top().d, Q.top().p, Q.top().dist, Q.top().cost, true);
    else return ResCami(false);
}

ResCami WSPSd(const Pos&i, bool (PLAYER_NAME::*potAnar)(const Pos&, int d),
              bool (PLAYER_NAME::*final)(const Pos&), int (PLAYER_NAME::*cost)(const Pos&), int maxd = INFINIT) 
{
    if ((this->*final)(i)) return ResCami(SENSE_DIR, i, 0, 0, true);
    PQRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;
    VVE C(MAX, VE(MAX, INFINIT));

    for (const Dir& d : _DA) { 
        Pos n = i + d;
        if ((this->*potAnar)(n, 1)) { 
            C[n.x][n.y] = (this->*cost)(n) + 10000; 
            Q.push(ResCami(d, n, 1, C[n.x][n.y])); 
    }   }

    while (not Q.empty() and not (this->*final)(Q.top().p)) {
        ResCami a = Q.top(); Q.pop();
        if (not S[a.p.x][a.p.y]) {
            S[a.p.x][a.p.y] = true;
            for (const Dir& d : _DA) { 
                Pos n = a.p + d;
                int c = (this->*cost)(n) + 10000;
                if ((this->*potAnar)(n, a.dist + 1) and 
                    C[a.p.x][a.p.y] + c < C[n.x][n.y]) {
                    C[n.x][n.y] = C[a.p.x][a.p.y] + c; 
                    if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist + 1, C[n.x][n.y]));
    }   }   }   }

    if (not Q.empty()) 
        return ResCami(Q.top().d, Q.top().p, Q.top().dist, Q.top().cost, true);
    else return ResCami(false);
}

ResCami WSPSd(const Pos&i, const Pos& f, bool (PLAYER_NAME::*potAnar)(const Pos&, int d),
              int (PLAYER_NAME::*cost)(const Pos&), int maxd = INFINIT) 
{
    if (i == f) return ResCami(SENSE_DIR, i, 0, 0, true);
    PQRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;
    VVE C(MAX, VE(MAX, INFINIT));

    for (const Dir& d : _DA) { 
        Pos n = i + d;
        if ((this->*potAnar)(n, 1)) { 
            C[n.x][n.y] = (this->*cost)(n) + 10000; 
            Q.push(ResCami(d, n, 1, C[n.x][n.y], C[n.x][n.y] + n.distancia(f))); 
    }   }

    while (not Q.empty() and Q.top().p != f) {
        ResCami a = Q.top(); Q.pop();
        if (not S[a.p.x][a.p.y]) {
            S[a.p.x][a.p.y] = true;
            for (const Dir& d : _DA) { 
                Pos n = a.p + d;
                int c = (this->*cost)(n) + 10000;
                if ((this->*potAnar)(n, a.dist + 1) and 
                    C[a.p.x][a.p.y] + c < C[n.x][n.y]) {
                    C[n.x][n.y] = C[a.p.x][a.p.y] + c; 
                    if (a.dist + 1 <= maxd) Q.push(ResCami(a.d, n, a.dist + 1, C[n.x][n.y], C[n.x][n.y] + n.distancia(f)));
    }   }   }   }

    if (not Q.empty()) 
        return ResCami(Q.top().d, Q.top().p, Q.top().dist, Q.top().cost, true);
    else return ResCami(false);
}

ResCami WSPHe(const Pos&i, int id, const Dir&oriei,
              bool (PLAYER_NAME::*potAnar)(const Pos&, int id),
              bool (PLAYER_NAME::*final)(const Pos&), int maxd = INFINIT)
{
    if ((this->*final)(i)) return ResCami(SENSE_DIR, SENSE_DIR, i, 0, 0, true);
    PQRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;
    VVE C(MAX, VE(MAX, INFINIT));

    for (const Dir& d : _DR) { 
        Pos n = i + d; S[n.x][n.y] = true;
        if ((this->*potAnar)(n, id)) { 
            C[n.x][n.y] = oriei.perpendicular(d) ? 2 : 1;
            Q.push(ResCami(d, SENSE_DIR, d, n, 1, C[n.x][n.y]));
    }   }

    PQRC Q2;

    while (not Q.empty() and not (this->*final)(Q.top().p)) {
        ResCami a = Q.top(); Q.pop();
        
        for (const Dir& d : _DR) {
            Pos n = a.p + d;
            int c = a.dant.perpendicular(d) ? 2 : 1;
            if (C[a.p.x][a.p.y] + c < C[n.x][n.y] and (this->*potAnar)(n, id)) {
                C[n.x][n.y] = C[a.p.x][a.p.y] + c;
                if (a.dist + 1 <= maxd) Q2.push(ResCami(a.d, d, d, n, a.dist + 1, C[n.x][n.y]));
    }   }   }

    if (not Q.empty()) return ResCami(Q.top().d, SENSE_DIR, Q.top().p, Q.top().dist, Q.top().cost, true);

    while (not Q2.empty() and not (this->*final)(Q2.top().p)) {
        ResCami a = Q2.top(); Q2.pop();
        if (not S[a.p.x][a.p.y]) {
            S[a.p.x][a.p.y] = true;
            for (const Dir& d : _DR) { 
                Pos n = a.p + d;
                int c = a.dant.perpendicular(d) ? 2 : 1;
                if (C[a.p.x][a.p.y] + c < C[n.x][n.y] and (this->*potAnar)(n, id)) {
                    C[n.x][n.y] = C[a.p.x][a.p.y] + c;
                    if (a.dist + 1 <= maxd) Q2.push(ResCami(a.d, a.d2, d, n, a.dist + 1, C[n.x][n.y]));
    }   }   }   }

    if (not Q2.empty()) return ResCami(Q2.top().d, Q2.top().d2, Q2.top().p, Q2.top().dist, Q2.top().cost, true);
    else return ResCami(false);
}

ResCami WSPHe(const Pos&i, const Pos& f, int id, const Dir&oriei,
              bool (PLAYER_NAME::*potAnar)(const Pos&, int id), int maxd = INFINIT)
{
    if (i == f) return ResCami(SENSE_DIR, SENSE_DIR, i, 0, 0, true);
    PQRC Q;
    VVB S(MAX, VB(MAX, false)); S[i.x][i.y] = true;
    VVE C(MAX, VE(MAX, INFINIT));

    for (const Dir& d : _DR) { 
        Pos n = i + d; S[n.x][n.y] = true;
        if ((this->*potAnar)(n, id)) { 
            C[n.x][n.y] = oriei.perpendicular(d) ? 2 : 1;
            Q.push(ResCami(d, SENSE_DIR, d, n, 1, C[n.x][n.y],  C[n.x][n.y] + n.distancia(f)));
    }   }

    PQRC Q2;

    while (not Q.empty() and Q.top().p != f) {
        ResCami a = Q.top(); Q.pop();
        
        for (const Dir& d : _DR) {
            Pos n = a.p + d;
            int c = a.dant.perpendicular(d) ? 2 : 1;
            if (C[a.p.x][a.p.y] + c < C[n.x][n.y] and (this->*potAnar)(n, id)) {
                C[n.x][n.y] = C[a.p.x][a.p.y] + c;
                if (a.dist + 1 <= maxd) Q2.push(ResCami(a.d, d, d, n, a.dist + 1, C[n.x][n.y], C[n.x][n.y] + n.distancia(f)));
    }   }   }

    if (not Q.empty()) return ResCami(Q.top().d, SENSE_DIR, Q.top().p, Q.top().dist, Q.top().cost, true);

    while (not Q2.empty() and Q2.top().p != f) {
        ResCami a = Q2.top(); Q2.pop();
        if (not S[a.p.x][a.p.y]) {
            S[a.p.x][a.p.y] = true;
            for (const Dir& d : _DR) { 
                Pos n = a.p + d;
                int c = a.dant.perpendicular(d) ? 2 : 1;
                if (C[a.p.x][a.p.y] + c < C[n.x][n.y] and (this->*potAnar)(n, id)) {
                    C[n.x][n.y] = C[a.p.x][a.p.y] + c;
                    if (a.dist + 1 <= maxd) Q2.push(ResCami(a.d, a.d2, d, n, a.dist + 1, C[n.x][n.y],  C[n.x][n.y] + n.distancia(f)));
    }   }   }   }

    if (not Q2.empty()) return ResCami(Q2.top().d, Q2.top().d2, Q2.top().p, Q2.top().dist, Q2.top().cost, true);
    else return ResCami(false);
}
virtual void play () {
	if (TORN == 0) _D.inicialitzaControl();
    permutaDirs(); llegirSoldats(); llegirHelis(); llegirObjs(); analitzaMapa();
    definirEstrategia(); dirigeixSoldats(); dirigeixHelis();
    _D.buidaDemandes();
}
}; RegisterPlayer(PLAYER_NAME);
