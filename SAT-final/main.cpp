#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

typedef vector<int> Clause;
typedef vector<Clause> CNF;
vector<int> assignment;


CNF read_file(const string &filename, int &n_vars) {
    ifstream fin(filename);
    string line;
    CNF cnf;
    while(getline(fin, line)) {
        if(line.empty() || line[0] == 'c') continue;
        if(line[0] == 'p') {
            string tmp;
            int n_clauses;
            istringstream iss(line);
            iss >> tmp >> tmp >> n_vars >> n_clauses;
            continue;
        }

        istringstream iss(line);
        int lit;
        Clause clause;
        while(iss >> lit) {
            if(lit == 0) {
                if(!clause.empty())
                    cnf.push_back(clause);
                clause.clear();
            } else {
                clause.push_back(lit);
            }
        }
    }

    return cnf;
}

/// *-*-*-*-*-*-*-*-*-*-*-*- Rezolutie *-*-*-*-*-*-*-*-*-*-*-*-*-*

//Funcție care caută un literal
bool literal_in_clause(const Clause &clause, int literal) {
    for(int i = 0; i < clause.size(); i ++) {
        if(clause[i] == literal) {
            return true;
        }
    }
    return false;
}

//Funcție care compară două clauze indiferent de ordine
bool equal_clauses(const Clause &c1, const Clause &c2) {
    if(c1.size() != c2.size()) {
        return false;
    }

    //verficam ca fiecare literal din c1 este in c2
    for(int i = 0; i < c1.size(); i ++) {
        if(!literal_in_clause(c2, c1[i])) {
            return false;
        }
    }
    return true;
}

//Funcție care verificaă dacă o clauză există deja în mulțimea de clauze(CNF)
bool clause_exists(const CNF &cnf, const Clause &c) {
    for(int i = 0; i < cnf.size(); i ++) {
        if(equal_clauses(c, cnf[i])) {
            return true;
        }
    }
    return false;
}

//Funcție care verifică dacă este o tautologie
bool is_tautology(const Clause &clause) {
    for (int i = 0; i < clause.size(); i++) {
        for (int j = i + 1; j < clause.size(); j++) {
            if (clause[i] == -1 * clause[j]) {
                return true;
            }
        }
    }
    return false;
}

//Rezolutia aplicata pe doua clauze
Clause resolve(const Clause &c1, const Clause &c2, bool &resolved) {
    resolved = false;
    for(int i = 0; i < c1.size(); i ++) {
        int lit1 = c1[i];
        for(int j = 0; j < c2.size(); j ++) {
            int lit2 = c2[j];
            if(lit1 == -1 * lit2) {
                resolved = true;
                Clause resolvent;
                for(int z = 0; z < c1.size(); z ++) {
                    if(c1[z] != lit1 && !literal_in_clause(resolvent, c1[z])) {
                        resolvent.push_back(c1[z]);
                    }
                }
                for(int z = 0; z < c2.size(); z ++) {
                    if(c2[z] != lit2 && !literal_in_clause(resolvent, c2[z])) {
                        resolvent.push_back(c2[z]);
                    }
                }

                if (is_tautology(resolvent)) {
                    resolved = false;
                    return Clause(); // Ignorăm tautologiile
                }

                return resolvent;
            }
        }
    }
    return Clause();
}

//Algoritm rezolutie
bool resolution(CNF &cnf) {
    bool added_new = true;
    while(added_new) {
        added_new = false;
        vector<Clause> new_clauses;
        for(int i = 0; i < cnf.size(); i ++) {
            for(int j = i + 1; j < cnf.size(); j ++) {
                bool resolved = false;
                Clause resolvent = resolve(cnf[i], cnf[j], resolved);

                if(resolved) {
                    if(resolvent.size() == 0) {
                        cout << "Clauza vida -> Formula este Nesatisfiabila" << endl;
                        return false;
                    }
                    if(!clause_exists(cnf, resolvent) && !clause_exists(new_clauses, resolvent)) {
                        new_clauses.push_back(resolvent);
                        added_new = true;
                    }
                }
            }
        }
        for(int i = 0; i < new_clauses.size(); i ++) {
            cnf.push_back(new_clauses[i]);
        }
    }
    cout << "Nu s-a gasit clauza vida. Nu se mai pot forma rezolventi. Formula este Satisfiabila." << endl;
    return true;
}


/// *-*-*-*-*-*-*-*-*-*-*-*- DP *-*-*-*-*-*-*-*-*-*-*-*-*-*

//Algoritm rezolutie pentru DP
CNF resolution_for_DP(CNF &clauses, bool &found_empty_clause) {
    bool added_new = true;
    CNF new_clauses = clauses;
    found_empty_clause = false;

    while(added_new) {
        added_new = false;
        vector<Clause> to_add;
        for(int i = 0; i < new_clauses.size(); i++) {
            for(int j = i + 1; j < new_clauses.size(); j++) {
                bool resolved = false;
                Clause resolvent = resolve(new_clauses[i], new_clauses[j], resolved);

                if(resolved) {
                    if(resolvent.empty()) {
                        found_empty_clause = true;
                        return {}; // întoarce o formulă vidă în caz de contradicție
                    }
                    if(!clause_exists(new_clauses, resolvent) && !clause_exists(to_add, resolvent)) {
                        to_add.push_back(resolvent);
                        added_new = true;
                    }
                }
            }
        }
        for(int i = 0; i < to_add.size(); i ++) {
            new_clauses.push_back(to_add[i]);
        }
    }

    return new_clauses;
}

//Funcție care verifică dacă există clauze unitare
bool find_unit_clause(const CNF &cnf, int &unit) {
    for(int i = 0; i < cnf.size(); i ++) {
        if(cnf[i].size() == 1) {
            unit = cnf[i][0];
            return true;
        }
    }
    return false;
}


//Funcție care verifică dacă un literal e în clauză
bool in_vector(const vector<int> &clause, int lit) {
    for(int i = 0; i < clause.size(); i++) {
        if(clause[i] == lit)
            return true;
    }
    return false;
}

//Funcție pentru găsire literal pur
bool find_pure_literal(const CNF &cnf, int &pure) {
    vector<int> seen_pos, seen_neg;
    for(int i = 0; i < cnf.size(); i ++) {
        for(int j = 0; j < cnf[i].size(); j ++) {
            int lit = cnf[i][j];
            if(lit > 0) {
                if(!in_vector(seen_pos, lit))
                    seen_pos.push_back(lit);
            } else {
                int abs_lit = -1 * lit;
                if(!in_vector(seen_neg, lit))
                    seen_neg.push_back(abs_lit);
            }
        }
    }

    for(int i = 0; i < seen_pos.size(); i ++) {
        int lit = seen_pos[i];
        if(!in_vector(seen_neg, lit)) {
            pure = lit;
            return true;
        }
    }

    for(int i = 0; i < seen_neg.size(); i ++) {
        int lit = seen_neg[i];
        if(!in_vector(seen_pos, lit)) {
            pure = lit;
            return true;
        }
    }

    return false;
}

//Funcție care elimină clauzele satisfăcute + șterge literalul opus
CNF simplify(const CNF& cnf, int literal) {
    CNF new_cnf;
    for(int i = 0; i < cnf.size(); i ++) {
        Clause clause;
        bool skip = false;
        for(int j = 0; j < cnf[i].size(); j ++) {
            if(cnf[i][j] == literal) {
                skip = true; //dacă e satisfăcută
                break;
            }
            if(cnf[i][j] != -1 * literal) {
                clause.push_back(cnf[i][j]);
            }
        }
        if(!skip && !clause_exists(new_cnf, clause)) {
            new_cnf.push_back(clause);
        }
    }
    return new_cnf;
}

//verfică dacă conține clauza vidă
bool contains_empty_clause(const CNF &cnf) {
    for(int i = 0; i < cnf.size(); i ++) {
        if(cnf[i].empty())
            return true;
    }
    return false;
}

CNF simplify_pure_literal(const CNF& cnf, int literal) {
    CNF new_cnf;
    for(int i = 0; i < cnf.size(); i ++) {
        Clause clause;
        for(int j = 0; j < cnf[i].size(); j ++) {
            if(cnf[i][j] != literal) {
                clause.push_back(cnf[i][j]);
            }
        }
        new_cnf.push_back(clause);
    }
    return new_cnf;
}

bool DP(CNF cnf) {
    while (true) {
        if (cnf.empty()) {
            cout << "Satisfiabil" << endl;
            return true;
        }

        if (contains_empty_clause(cnf)) {
            cout << "Nesatisfiabil" << endl;
            return false;
        }

        int literal;

        if (find_unit_clause(cnf, literal)) {
            cnf = simplify(cnf, literal);
            continue;
        }

        if (find_pure_literal(cnf, literal)) {
            cnf = simplify(cnf, literal);
            continue;
        }

        /// Când nu mai avem ce reduce, aplicăm rezoluția:
        bool found_empty = false;
        CNF resolved_cnf = resolution_for_DP(cnf, found_empty);

        if (found_empty) {
            cout << "Clauza vida dupa rezolutie -> Nesatisfiabil" << endl;
            return false;
        }

        if (resolved_cnf == cnf) {
            cout << "Nicio schimbare dupa rezolutie -> Satisfiabil" << endl;
            return true;
        }

        // se reia DP pe noua formulă obținută
        cnf = resolved_cnf;
    }
}


/// *-*-*-*-*-*-*-*-*-*-*-*- DPLL *-*-*-*-*-*-*-*-*-*-*-*-*-*
int chooseVariableJW(const CNF &cnf, const vector<int> &assignment) {
    vector<double> score(assignment.size(), 0.0);

    for (const Clause &clause : cnf) {
        int len = clause.size();
        double weight = pow(2.0, -len);
        for (int lit : clause) {
            int var = abs(lit);
            if (assignment[var] == 0) {
                score[var] += weight;
            }
        }
    }

    int bestVar = -1;
    double bestScore = -1.0;
    for (int var = 1; var < score.size(); ++var) {
        if (assignment[var] == 0 && score[var] > bestScore) {
            bestScore = score[var];
            bestVar = var;
        }
    }

    return bestVar;
}

bool unitPropagate(const CNF &cnf, vector<int> &assignment, int n) {
    bool changed = true;
    while(changed) {
        changed = false;
        for(int i = 0; i < cnf.size(); i ++) {
            int numUnassigned = 0;
            int lastUnassignedLit = 0;
            bool satisfied = 0;
            for(int j = 0; j < cnf[i].size(); j ++) {
                int val = assignment[abs(cnf[i][j])];
                if((cnf[i][j] > 0 && val == 1) || (cnf[i][j] < 0 && val == -1)) {
                    satisfied = true;
                    break;
                } else if(val == 0) {
                    numUnassigned ++;
                    lastUnassignedLit = cnf[i][j];
                }
            }
            if(satisfied == false) {
                if(numUnassigned == 0)
                    return false;
                if(numUnassigned == 1) {
                    if(lastUnassignedLit > 0)
                        assignment[abs(lastUnassignedLit)] = 1;
                    else
                        assignment[abs(lastUnassignedLit)] = -1;
                    changed = true;
                }
            }
        }
    }
    return true;
}

bool DPLL(const CNF &cnf, vector<int> &assignment, int n) {
    if(unitPropagate(cnf, assignment, n) == false)
        return false;
    bool allClausesSatisfied = true;
    for(int i = 0; i < cnf.size(); i ++) {
        bool clauseSatisfied = false;
        for(int j = 0; j < cnf[i].size(); j ++) {
            int val = assignment[abs(cnf[i][j])];
            if((cnf[i][j] > 0 && val == 1) || (cnf[i][j] < 0 && val == -1)) {
                clauseSatisfied = true;
                break;
            }
        }
        if(clauseSatisfied == false) {
            allClausesSatisfied = false;
            break;
        }
    }
    if(allClausesSatisfied == true)
        return true;
    for(int var = 1; var <= n; var++) {
        if(assignment[var] == 0) {
            vector<int> saved = assignment;
            assignment[var] = 1;
            if (DPLL(cnf, assignment, n))
                return true;
            assignment = saved;
            assignment[var] = -1;
            if (DPLL(cnf, assignment, n))
                return true;

            assignment = saved;
            return false;
        }
    }
    return false;
}


//Poți folosi meniul pentru testare sau direct unul dintre algoritmi eliminând simbolurile de comentariu
int main()
{
    int n;
    CNF cnf = read_file("text.in", n);

    vector<int> assignment(n + 1, 0);

    /*
    int choice;
    cin >> choice;
    switch(choice) {
        case 1:
            resolution(cnf);
            break;
        case 2:
            DP(cnf);
            break;
        case 3:
            if(DPLL(cnf, assignment, n) == true) {
                cout << "Satisifiabila" << endl;
                for(int i = 1; i <= n; i ++)
                    cout << i << ": " << assignment[i] << endl;
            } else {
                cout << "Nesatisfiabila" << endl;
            }
    } */

    //resolution(cnf);

    //DP(cnf);

    /*
    if(DPLL(cnf, assignment, n) == true) {
        cout << "Satisifiabila" << endl;
        for(int i = 1; i <= n; i ++)
            cout << i << ": " << assignment[i] << endl;
    } else {
        cout << "Nesatisfiabila" << endl;
    }*/

    return 0;
}

