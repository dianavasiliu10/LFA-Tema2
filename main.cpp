#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

unsigned int n,m,q0,st,fn;
char chr;
vector<char> alfabet;
vector<vector<int>> lambdaClosure;
vector<int> alpha;
vector<int> stFin1;
vector<int> stFin2;
vector<vector<int>> doi;
vector<vector<vector<int>>> delta;
vector<int> viz;

unsigned getPos(char c)
{
    for (unsigned i=0; i<alfabet.size(); i++)
        if (c==alfabet[i])
            return i;
    return -1;
}

void input(vector<vector<vector<int>>>& d)
{
    const string filename = "input.txt";
    ifstream fin(filename);
    unsigned i,k;
    fin>>n>>m;
    for (i=0; i<m; i++) {
        fin>>chr;
        alfabet.push_back(chr);
        doi.push_back(stFin1); // creez "doi" ca linie in delta si delta2, adica vector de vectori
    }
    alfabet.push_back('$');
    doi.push_back(stFin1); // adaug o coloana, pentru delta (coloana lambda)

    fin>>q0>>m;
    for (i=0; i<m; i++) {
        fin>>k;
        stFin1.push_back(k); // creez vector pentru starile finale initiale
    }

    for (i=0; i<n; i++) {
        d.push_back(doi);   // creez matricea delta
    }

    fin>>m;
    for(i=0; i<m; i++) {
        fin>>st>>chr>>fn;
        d[st][getPos(chr)].push_back(fn); // creez matricea delta (a automatului initial)
    }

    fin.close();
}

void print(vector<vector<vector<int>>> d)
{
    for (unsigned i = 0; i < n; i++) {
        cout << "i= " << i <<"\n";
        for (unsigned j = 0; j < d[i].size(); j++) {
            cout<<"  j="<<j<<":   ";
            for (unsigned k : d[i][j])
                cout << k << " ";
            cout << "\n";
        }
        cout << "\n";
    }
}

bool isInV(unsigned x, const vector<int>& v)
{
    for (unsigned i : v)
        if (x==i)
            return true;
    return false;
}

void createLambdaClosure(vector<vector<int>>& v)
{
    int s,f;
    for (unsigned i=0; i<n; i++) {
        v.push_back(stFin2);        // v este tabelul lambda-closure
        unsigned nod = i;
        v[i].push_back(nod);
        s = 0;
        f = 0;
        while (s <= f) {
            for (int j : delta[nod][alfabet.size() - 1]) {      //j este fiecare element de pe linia curenta, coloana lui lambda
                if (!isInV(j, v[i])) {      //daca j nu exista in v[i] (in lambdaClosure[i]), il adaug
                    v[i].push_back(j);
                    f++;
                }
            }
            s++;
            nod = v[i][s];
        }
    }
}

void createAlpha(vector<int>& a, unsigned j, unsigned nod)
{
    for (int i : lambdaClosure[nod]) {
        for (int x : delta[i][j])       // j este coloana literei curente
            if (!isInV(x,a))    //daca x nu este in a (alpha), il adaug
                a.push_back(x);
    }
}

void uni(const vector<int>& a, vector<int>& b)
{
    for (int j : a)
        if (!isInV(j,b))
            b.push_back(j);
}

void createDelta2(vector<int>& v)
{
    for (int i : alpha)
        if (!isInV(i,v))
            uni(lambdaClosure[i], v);       //lambda-closeure-ul fiecarui nod din alpha U v
}

bool equalArrays(const vector<int>& a, vector<int> b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (int i : a) {
        int ok=1;
        for (unsigned j = 0; j < b.size() && ok; j++) {
            if (i == b[j]) {
                b[j] = -1;
                ok = 0;
            }
        }
        if (ok) {      // nu a gasit elementul i in b
            return false;
        }
    }
    return true;
}

vector<vector<vector<int>>> lnfa_to_nfa(vector<vector<vector<int>>> v)
{
    createLambdaClosure(lambdaClosure);     // creez lambda-inchiderea pt toate starile

    for (unsigned i=0; i<n; i++) {
        v[i].pop_back();    // sterg coloana lui lambda de pe linia i
        for (unsigned j=0; j<alfabet.size() - 1; j++) {
            createAlpha(alpha, j, i);      // creez tranzitia cu caracterul alpha
            v[i][j].clear();
            createDelta2(v[i][j]);      // creez tranzitia finala cu caracterul alpha (va fi in d*)

            alpha.clear();
        }
    }

    for (unsigned i=0; i<n; i++) {
        for (int j : stFin1)
            if (isInV(j,lambdaClosure[i])) {
                stFin2.push_back(i);        // creez vectorul de stari finale
                break;
            }
    }

    vector<int> sters(n);

    for (unsigned i=0; i<n-1; i++) {
        if (sters[i]==0) {
            unsigned j = i + 1;
            while (j < n) {
                int ok = 1;
                if (isInV(i, stFin2) == isInV(j, stFin2)) {  // daca ambele sunt stari finale/nefinale
                    for (unsigned k = 0; k < alfabet.size() - 1 && ok; k++)
                        if (!equalArrays(v[i][k],v[j][k]))         // daca gasesc o coloana pe care cele 2 linii sunt diferite
                            ok = 0;

                    if (ok != 0) { // liniile i si j sunt egale ca vectori
                        sters[j] = i + 1;
                        for (unsigned k=0; k<alfabet.size()-1; k++)
                            v[j][k].clear();

                        v[j].clear();
                    }
                }
                j++;
            }
        }
    }

    // schimb delta eliminand/inlocuind toate starile care trebuie eliminate
    for (unsigned i=0; i<n; i++) {
        if (sters[i] == 0) {
            for (unsigned j = 0; j < alfabet.size() - 1; j++) {
                unsigned k = 0;
                while (k < v[i][j].size()) {
                    int x = v[i][j][k];

                    if (sters[x] != 0) {
                        v[i][j][k] = -1;
                        if (isInV(sters[x] - 1, v[i][j])) {     // daca elementul cu care vreau sa inlocuiesc exista deja in vector
                            v[i][j].erase(v[i][j].begin() + k);     //il sterg
                            k--;
                        }
                        else
                            v[i][j][k] = sters[x] - 1;      //altfel, il inlocuiesc
                    }
                    k++;
                }
            }
        }
    }

    for (unsigned i=0; i<stFin2.size(); i++)
        if (sters[stFin2[i]] != 0)
            stFin2.erase(stFin2.begin() + i);

    for (unsigned i=0; i<n; i++) {
        if (sters[i] == 0) {
            for (unsigned j=0; j<alfabet.size() - 1; j++)
                sort(v[i][j].begin(), v[i][j].end());
        }
    }
    alfabet.pop_back();
    return v;
}

bool isInS(const string& s, const vector<string>& v)
{
    for (const string& a : v)
        if (a == s)
            return true;
    return false;
}

unsigned posOfString(const string& s, vector<string> v)
{
    for (unsigned i=0; i<v.size(); i++)
        if (s == v[i])
            return i;
    return -1;
}

bool isInQueue(char c, const string& s)
{
    for (char i : s)
        if (c == i)
            return true;
    return false;
}

vector<vector<vector<int>>> nfa_to_dfa(vector<vector<vector<int>>> v)
{
    vector<string> coada;
    string str;
    vector<vector<string>> deltaS;
    string rez;
    vector<string> unu;
    vector<vector<int>> vfin;
    vector<int> oneInt;

    deltaS.push_back(unu);
    coada.push_back(to_string(q0));
    st=0; fn=0;

    while (st<=fn) {
        str = coada[st];
        if (str.size() == 1) {       // daca starea este formata dintr-un singur nod
            int nod = str[0] - '0';
            for (unsigned i=0; i<alfabet.size(); i++) {
                for (unsigned j : v[nod][i])
                    rez += to_string(j);        // rezultatul va fi string-ul format din elementele de pe linia lui

                if (!isInS(rez,coada)) {
                    deltaS.push_back(unu);
                    coada.push_back(rez);       // daca nu este in coada, il adaug
                    fn++;
                }

                deltaS[st].push_back(rez);      //matrice de string-uri
                rez = "";
            }
            st++;
        }
        else {
            int nod = str[0] - '0';
            for (unsigned i=0; i<alfabet.size(); i++) {
                for (unsigned cfr=1; cfr<str.size(); cfr++) {
                    int x = str[cfr] - '0';
                    uni(v[x][i], v[nod][i]);    // fac reuniunea tuturor vectorilor din delta de pe coloana i pt starile din str; reuniunea se face in v[nod][i]
                }

                sort(v[nod][i].begin(), v[nod][i].end());       // sortez pentru a avea o stare mai usor de vizualizat

                for (unsigned k : v[nod][i])
                    rez += to_string(k);        // creez string-ul corespunzator starii

                if (!isInS(rez,coada)) {     // daca rezultatul nu este in coada, il adaug
                    deltaS.push_back(unu);
                    coada.push_back(rez);
                    fn++;
                }

                deltaS[st].push_back(rez);
                rez = "";
            }
            st++;
        }
    }

    v.clear();
    n = st;     //am un nou numar de stari
    doi.clear();

    //redenumesc starile cu pozitiile lor din coada
    for (unsigned i=0; i<n; i++) {
        v.push_back(doi);
        for (unsigned j=0; j<alfabet.size(); j++) {
            v[i].push_back(alpha);
            v[i][j].push_back(posOfString(deltaS[i][j], coada));
        }
    }

    stFin1.clear();
    for (unsigned i=0; i<n; i++){        // verific fiecare stare daca este finala
        for (int j : stFin2) {
            if (isInQueue(to_string(j)[0], coada[i])) {    // daca starea curenta i contine o stare finala initiala
                stFin1.push_back(i);        // o adaug la starile finale
                break;
            }
        }
    }

    return v;
}


void dfs(unsigned x, vector<vector<vector<int>>> v)
{
    if (viz[x] == 0) {
        viz[x] = 1;
        for (unsigned i=0; i<alfabet.size(); i++) {
            if (!v[x][i].empty()) {
                unsigned nod = v[x][i][0];
                dfs(nod,v);
            }
        }
    }
}

vector<vector<vector<int>>> dfa_to_dfamin(vector<vector<vector<int>>> v)
{
    vector<vector<int>> eqv;
    unsigned i,j,k,x,y;

    //punctul 1
    for (i=0; i<n; i++)
        eqv.push_back(alpha);

    for (i=1; i<n; i++)
        for (j=0; j<i; j++)
            eqv[i].push_back(1);

    //punctul 2
    for (i=1; i<n; i++)
        for (j=0; j<i; j++)
            if ((isInV(i,stFin1) && !isInV(j,stFin1)) || (!isInV(i,stFin1) && isInV(j,stFin1)))
                eqv[i][j] = 0;

    int ok2;
    ok2=1;
    while(ok2) {        // punctul 4
        ok2=0;
        for (j=1; j<n; j++) {       //linia q
            for (k=0; k<j; k++) {           //coloana r
                if (eqv[j][k] == 1) {       // (j,k) este perechea (q,r)
                    for (i = 0; i < alfabet.size(); i++) {
                        x = v[j][i][0];         //(x,y) este perechea (delta(q,i),delta(r,i))
                        y = v[k][i][0];
                        if (x!=y) {
                            if (x < y) {        //coloana trebuie sa fie < decat linia (pt diagonala)
                                unsigned aux = x;
                                x = y;
                                y = aux;
                            }

                            //punctul 3
                            //verific simetricul fata de diagonala sa fie egal cu elementul pe care sunt
                            if (eqv[x][y] == 0) {           // daca perechea e marcata cu FALSE,
                                eqv[j][k] = 0;              // transform si perechea (q,r) in FALSE
                                ok2=1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    vector<int> equals(n);      //vector care retine echivalenta intre stari
    unsigned nr=0;

    for (i=0; i<n-1; i++) {
        if (equals[i] == 0) {
            nr++;
            equals[i] = nr;
            for (j = i + 1; j < n; j++) {    // se verifica echivalenta intre starile i si j
                if (isInV(i, stFin1) == isInV(j, stFin1)) {       // daca ambele sunt stari finale/nefinale
                    ok2 = 1;
                    for (k=0; k<n && ok2; k++) {        //verific intreaga linie
                        if (k<i) {           //inainte de diagonala principala
                            if (k<j) {
                                if (eqv[i][k] != eqv[j][k]) {
                                    ok2=0;
                                }
                            }

                        }
                        if (k==i) {         //pe diagonala principala
                            if (k<j) {      // adevarat mereu
                                if (eqv[j][k] != 1) {       //pt ca pe diagonala principala trebuie sa fie true
                                    ok2=0;
                                }
                            }
                        }
                        if (k>i) {          //dupa diagonala principala
                            if (k<j) {      //daca inca exista elemente pe linia lui j
                                if (eqv[k][i] != eqv[j][k]) {       //verific simetricul lui i cu j
                                    ok2=0;
                                }
                            }
                            if (k==j) {     //j,k este true
                                if (eqv[k][i] != 1) {       //deci si k,i trebuie sa fie true
                                    ok2=0;
                                }
                            }
                            if (k>j) {      //daca niciuna din valori nu mai exista
                                if (eqv[k][i] != eqv[k][j]) {       //verific simetricele
                                    ok2=0;
                                }
                            }
                        }
                    }
                    if (ok2 == 1)       //liniile sunt egale
                        equals[j] = nr;
                }
            }
        }
    }

    if (equals[n-1] == 0) {     //inseamna ca nu a fost egal cu nicio alta linie
        nr++;
        equals[n-1] = nr;
    }

    vector<vector<vector<int>>> rez;
    for (i=0; i<nr; i++)
        rez.push_back(doi);

    //redenumirea starilor
    i=0;
    for (k=0; k<nr; k++) {
        while (equals[i] != k+1)
            i++;

        for (j=0; j<alfabet.size(); j++) {
            x = delta[i][j][0];        // elementul in care se duce i cu litera j
            rez[k].push_back(alpha);
            rez[k][j].push_back(equals[x]-1);       //echivalentul lui (redenumit)
        }
    }

    q0 = equals[q0]-1;

    stFin2.clear();
    for (unsigned i : stFin1) {
        if (!isInV(equals[i]-1,stFin2)) {
            stFin2.push_back(equals[i]-1);
        }
    }

    n=nr;
    vector<int> sters;

    viz.clear();
    viz.resize(n);
    for (int nod=0; nod<n; nod++) {
        dfs(nod, rez);
        ok2=1;
        for (i=0; i<n && ok2; i++) {
            if (viz[i] == 1 && isInV(i, stFin2))        //daca ajung cu nodul nod intr-o stare finala, e ok
                ok2 = 0;
        }
        if (ok2)
            sters.push_back(nod);       //altfel, il sterg

        viz.clear();
        viz.resize(n);
    }

    viz.clear();
    viz.resize(n);
    dfs(q0,rez);

    for (i=0; i<n; i++)
        if (viz[i] == 0)        //inseamna ca i este inaccesibil, deci il sterg
            sters.push_back(i);

    v.clear();
    vector<int> unu;

    //creez matricea finala
    nr=0;
    for (i=0; i<n; i++) {
        if (!isInV(i,sters)) {      //daca i nu e sters
            v.push_back(doi);       //adaug linie pentru el
            for (j = 0; j < alfabet.size(); j++)
                v[nr].push_back(unu);       //adaug coloana pe linia i pentru fiecare litera din alfabet

            for (j=0; j<alfabet.size(); j++) {
                v[nr][j].push_back(rez[i][j][0]);    //copiez informatia din rez de pe linia corespunzatoare
            }
        }
        nr++;
    }

//    j=0;
//    for (i=0; i<n; i++) {
//        if (!isInV(i,sters)) {
//            for (k=0; k<alfabet.size(); k++) {
//                v[j][k].push_back(rez[i][k][0]);
//            }
//            j++;
//        }
//    }

    n=nr;

    return v;
}

int main()
{
    input(delta);

    delta = lnfa_to_nfa(delta);

    print(delta);

    cout<<"============================\n";

    delta = nfa_to_dfa(delta);

    print(delta);

    cout<<"============================\n";

    delta = dfa_to_dfamin(delta);

    print(delta);

    return 0;
}
