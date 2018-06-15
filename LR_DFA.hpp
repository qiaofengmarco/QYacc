#ifndef LR_DFA_HPP
#define LR_DFA_HPP

#include "Headers.h"
#include "Preprocess.hpp"

#define SHIFT -1
#define ACC   -2

hash<string> str_hasher;
hash<int> int_hasher;

struct mypair
{
    int first, second;
    size_t hash_val;
    mypair(int a, int b)
    {
        string str = "";
        char* c = (char*) malloc(sizeof(char) * 5), *d = (char*) malloc(sizeof(char) * 5);
        first = a;
        second = b;
        _itoa(a, c, 10);
        str = c;
        str += " ";
        _itoa(b, d, 10);
        str += d;
        hash_val = str_hasher(str);
        free(c);
        free(d);
        c = NULL;
        d = NULL;
    }
    mypair(const mypair& a)
    {
        first = a.first;
        second = a.second;
        hash_val = a.hash_val;
    }
    mypair operator= (const mypair&a)
    {
        first = a.first;
        second = a.second;
        hash_val = a.hash_val;
    }
    bool operator==(const mypair& a)
    {
        return ((first == a.first) && (second == a.second));
    }
    bool operator==(const mypair& a) const
    {
        return ((first == a.first) && (second == a.second));
    }
};

template<>
struct hash < mypair >
{
    size_t operator()(const mypair &a)
    {
        return a.hash_val;
    }
};

//set union operation
template<typename T>
bool setUnion(unordered_set<T> &a, const unordered_set<T> &b)
{
    bool change = false;
    pair<unordered_set<T>::iterator, bool> pair;
    for (unordered_set<T>::iterator i = b.begin(); i != b.end(); i++)
    {
        pair = a.insert(*i);
        change = change || pair.second;
    }
    return change;
}

struct PItem
{
    int productionID, lookAhead, dot;
    size_t hash_val;
    PItem(int p, int l, int d)
    {
        string str = "";
        char *a = (char*) malloc(sizeof(char) * 5), *b = (char*) malloc(sizeof(char) * 5), *c = (char*) malloc(sizeof(char) * 5);
        productionID = p;
        lookAhead = l;
        dot = d;
        _itoa(p, a, 10);
        str = a;
        str += " ";
        _itoa(l, b, 10);
        str += b;
        str += " ";
        _itoa(d, c, 10);
        str += c;
        hash_val = str_hasher(str);
        free(a);
        free(b);
        free(c);
        a = NULL;
        b = NULL;
        c = NULL;
    }
    PItem(const PItem &a)
    {
        productionID = a.productionID;
        lookAhead = a.lookAhead;
        dot = a.dot;
        hash_val = a.hash_val;
    }
    PItem operator= (const PItem &a)
    {
        productionID = a.productionID;
        lookAhead = a.lookAhead;
        dot = a.dot;
        hash_val = a.hash_val;
    }
    bool operator==(const PItem &a)
    {
        return ((productionID == a.productionID) && (dot == a.dot) && (lookAhead == a.lookAhead));
    }
    bool operator==(const PItem &a) const
    {
        return ((productionID == a.productionID) && (dot == a.dot) && (lookAhead == a.lookAhead));
    }
    bool operator!=(const PItem &a)
    {
        return ((productionID != a.productionID) || (dot != a.dot) || (lookAhead != a.lookAhead));
    }
    bool operator!=(const PItem &a) const
    {
        return ((productionID != a.productionID) || (dot != a.dot) || (lookAhead != a.lookAhead));
    }
};

template<>
struct hash < PItem >
{
    size_t operator()(const PItem &a)
    {
        return a.hash_val;
    }
};

struct DFA_Item
{
    unordered_set<PItem> core;
    unordered_set<PItem> component;
    int id;
    struct link
    {
        struct DFA_Item* To;
        bool shift_goto_non_empty;
        int action;
        link()
        {
            shift_goto_non_empty = false;
            action = SHIFT;
        }
    }; 
    unordered_map<int, link> Shift_Goto_Reduce;
};

class LR_DFA
{
public:
    LR_DFA()
    {
        if (!init_flag)
        {
            init_flag = true; 
            mapTokenProcess();
            yProcess();
            pdProcess();
            total_action = 130 + TokenAndLabel.size();
        }
    };
    void construct()
    {
        queue< unordered_set<PItem> > next_cores;
        queue< unordered_set<mypair> > next_LR0_cores;
        queue<int> ItemIndex_Queue, next_actions;
        unordered_set<int> after_dot;
        unordered_set<PItem> new_core;
        unordered_set<mypair> new_LR0_core;
        vector<mypair> Reduce;
        int pid, dot, lookAhead, right_size, this_action, index, Item_Index;
        DFA_Item **s = new DFA_Item*;
        vector< unordered_set< mypair> > core_item_map;
        bool change_core;

        cout << "Start constructing LALR(1) DFA..." << endl;

        start = new DFA_Item();
        start->id = 0;
        int k = firstPos.at(start_id);
        do
        {
            start->core.emplace(k, '$', 0);
            new_LR0_core.emplace(k, 0);
            k++;
        }
        while ((k < productions.size()) && (productions[k].left == start_id));
        core_item_map.emplace_back(new_LR0_core);
        states.push_back(start);
        ItemIndex_Queue.push(0);
        new_LR0_core.clear();

        //Use breath-first traverse method to construct cores
        //It could successfully construct LALR DFA
        while (!ItemIndex_Queue.empty())
        {
            Item_Index = ItemIndex_Queue.front();
            ItemIndex_Queue.pop();

            *s = states[Item_Index];

            closure((*s)->core, (*s)->component);

            after_dot.clear();
            Reduce.clear();
            for (unordered_set<PItem>::iterator i = (*s)->component.begin(); i != (*s)->component.end(); i++)
            {
                pid = (*i).productionID;
                dot = (*i).dot;
                lookAhead = (*i).lookAhead;
                right_size = productions[pid].right.size();

                //for reduce
                if (dot == right_size)
                {
                    Reduce.emplace_back(lookAhead, pid);
                    continue;
                }

                //for shift and goto
                this_action = productions[pid].right[dot];
                after_dot.insert(this_action);
            }

            //for reduce actions
            for (unsigned int i = 0; i < Reduce.size(); i++)
                (*s)->Shift_Goto_Reduce[Reduce[i].first].action = Reduce[i].second;

            for (unordered_set<int>::iterator i = after_dot.begin(); i != after_dot.end(); i++)
            {
                for (unordered_set<PItem>::iterator j = (*s)->component.begin(); j != (*s)->component.end(); j++)
                {
                    pid = (*j).productionID;
                    dot = (*j).dot;
                    lookAhead = (*j).lookAhead;

                    right_size = productions[pid].right.size();

                    if (dot < right_size)
                    {
                        this_action = productions[pid].right[dot];
                        if (this_action == *i)
                        {
                            new_core.emplace(pid, lookAhead, dot + 1);
                            new_LR0_core.emplace(pid, dot + 1);
                        }
                    }
                }

                if (new_core.empty())
                    index = -1;
                else
                    index = LR0_core_index_item(new_LR0_core, core_item_map);

                //the LR(0) DFA core does not exist
                if (index < 0)
                {
                    if (!new_core.empty())
                    {
                        next_cores.push(new_core);
                        next_LR0_cores.push(new_LR0_core);
                        next_actions.push(*i);
                    }
                }
                //the LR(0) DFA core exist
                else
                {
                    change_core = setUnion((*states[index]).core, new_core);
                    setUnion((*states[index]).component, new_core);
                    (*s)->Shift_Goto_Reduce[*i].shift_goto_non_empty = true;
                    (*s)->Shift_Goto_Reduce[*i].To = states[index];
                    if (change_core)
                        ItemIndex_Queue.push(index);
                }

                new_core.clear();
                new_LR0_core.clear();
            }

            while (!next_cores.empty())
            {
                new_core = next_cores.front();
                new_LR0_core = next_LR0_cores.front();
                this_action = next_actions.front();
                next_cores.pop();
                next_actions.pop();
                next_LR0_cores.pop();
                if (!(*s)->Shift_Goto_Reduce[this_action].shift_goto_non_empty)
                {
                    (*s)->Shift_Goto_Reduce[this_action].shift_goto_non_empty = true;
                    (*s)->Shift_Goto_Reduce[this_action].To = new DFA_Item();
                    setUnion((*s)->Shift_Goto_Reduce[this_action].To->core, new_core);
                    (*s)->Shift_Goto_Reduce[this_action].To->id = ids;
                    core_item_map.emplace_back(new_LR0_core);
                    states.push_back((*s)->Shift_Goto_Reduce[this_action].To);
                    ItemIndex_Queue.push(ids);
                    ids++;
                }
            }
            new_core.clear();
            new_LR0_core.clear();
        }

        cout << "Finished constructing LALR(1) DFA." << endl;

        delete s;
        unordered_set<PItem>().swap(new_core);
        unordered_set<mypair>().swap(new_LR0_core);
        unordered_set<int>().swap(after_dot);
        queue< unordered_set<PItem> >().swap(next_cores);
        queue< unordered_set<mypair> >().swap(next_LR0_cores);
        queue<int>().swap(ItemIndex_Queue);
        queue<int>().swap(next_actions);
        vector< unordered_set< mypair> >().swap(core_item_map);
    }

    void printTable()
    {
        int count;
        unsigned int states_size = states.size(), this_size, productions_size = productions.size();
        ofstream out("./y.tab.c", ios::app);

        cout << "Start printing orginal productions..." << endl;
        out << "int production_count = " << productions_size << ";" << endl;
        out << "int production_label_count[" << productions_size << "] = {";
        out << productions[0].right.size() << ", ";
        unsigned int max_length = productions[0].value.size();
        for (unsigned int i = 1; i < productions_size; i++)
        {
            out << productions[i].right.size();
            if (i < productions_size - 1)
                out << ", ";
            if (max_length < productions[i].value.size())
                max_length = productions[i].value.size();
        }
        out << "};" << endl;
        out << "int production_left[" << productions_size << "] = {";
        for (unsigned int i = 0; i < productions_size; i++)
        {
            out << productions[i].left;
            if (i < productions_size - 1)
                out << ", ";
        }
        out << "};" << endl;
        out << "char productions[" << productions_size << "][" << max_length << "] = {" << endl;
        for (unsigned int i = 0; i < productions_size; i++)
        {
            out << "    \"";
            for (unsigned int j = 0; j < productions[i].value.size(); j++)
            {
                if (productions[i].value[j] == '\'')
                    out << "\\";
                out << productions[i].value[j];
            }
            out << "\"";
            if (i < productions_size - 1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        cout << "Finished printing original productions" << endl;

        cout << "Start printing Parse Table..." << endl;
        out << "int parser_total_action = " << total_action << ";" << endl;
        out << "int tokenAmount = " << tokenAmount << ";" << endl;
        out << "int parser_state_num = " << states_size << ";" << endl;
        out << "int parser_action_count[" << states_size << "] = {";
        for (unsigned int i = 0; i < states_size; i++)
        {
            out << states[i]->Shift_Goto_Reduce.size();
            if (i < states_size - 1)
                out << ", ";
        }
        out << "};" << endl;
        out << "int* parser_action[" << states_size << "];" << endl;
        out << "int* parser_table[" << states_size << "];" << endl;
        out << "void table_action_init() {" << endl;
        for (unsigned int i = 0; i < states_size; i++)
        {
            this_size = states[i]->Shift_Goto_Reduce.size();
            out << "    parser_action[" << i << "] = (int*)malloc(sizeof(int) * " << this_size << ");" << endl;
            out << "    parser_table[" << i << "]  = (int*)malloc(sizeof(int) * " << this_size << ");" << endl;
            count = 0;
            for (unordered_map<int, DFA_Item::link>::iterator j = states[i]->Shift_Goto_Reduce.begin(); j != states[i]->Shift_Goto_Reduce.end(); j++, count++)
            {
                out << "    parser_action[" << i << "][" << count << "] = " << (*j).first << ";" << endl;
                out << "    parser_table[" << i << "][" << count << "]  = ";
                if ((*j).second.shift_goto_non_empty)
                    out << (*j).second.To->id;
                else
                    out << (*j).second.action + states_size;
                out << ";" << endl;
            }
        }
        out << "}" << endl;
        cout << "Finished printing Parse Table." << endl;
        out.close();
    }

    DFA_Item* start;
    int state_id;
    vector<DFA_Item*> states;
    static int total_action;
    static bool init_flag;
    static int ids;
private:
    void closure(const unordered_set<PItem> &core, unordered_set<PItem> &component)
    {
        setUnion(component, core);
        bool change = false, has_beta = false;
        int pid, dot, lookAhead, temp_int, beta, k, this_left;
        unordered_set<int> temp_seq;
        unordered_map<int, int>::iterator it;
        pair<unordered_set<PItem>::iterator, bool> pair;
        unordered_map<PItem, bool> explored;
        unordered_map<PItem, bool>::iterator map_it;
        do
        {
            change = false;
            for (unordered_set<PItem>::iterator i = component.begin(); i != component.end(); i++)
            {
                map_it = explored.find(*i);
                if (map_it != explored.end()) continue;
                explored[*i] = true;

                has_beta = false;
                pid = (*i).productionID;
                dot = (*i).dot;
                lookAhead = (*i).lookAhead;
                
                temp_int = productions[pid].right.size() - 1;

                if (dot > temp_int) //A->aBC. ignoring
                    continue;
                else if (dot < temp_int) //A->a.BC
                {
                    has_beta = true;
                    beta = productions[pid].right[dot + 1];
                }

                this_left = productions[pid].right[dot];
                it = firstPos.find(this_left);

                if (it == firstPos.end()) continue;

                //first(beta + a)
                if (has_beta)
                {
                    //first(beta)
                    first(beta, temp_seq);
                    
                    //if (beta == epsilon || beta is nullable)
                    //  first(beta + a) = first(beta) unions first(a)
                    if ((beta == 129) || ((beta >= tokenAmount) && (nullable.at(beta))))
                        temp_seq.insert(lookAhead);
                }
                else //first(beta + a) = first(a) = a
                    temp_seq.insert(lookAhead);
                
                //k = firstPos[this_left]
                k = (*it).second;
                do
                {
                    for (unordered_set<int>::iterator act = temp_seq.begin(); act != temp_seq.end(); act++)
                    {
                        pair = component.emplace(k, *act, 0);
                        change = change || pair.second;
                    }
                    k++;
                }
                while ((k < productions.size()) && (productions[k].left == this_left));
                temp_seq.clear();
            }
        }
        while (change);
        
        unordered_set<int>().swap(temp_seq);
        unordered_map<PItem, bool>().swap(explored);
    }

    void first(const int id, unordered_set<int> &ans)
    {
        if ((id < tokenAmount) && (id >= 0) && (id != 128))
        {
            ans.insert(id);
            return;
        }
        int fpos = firstPos[id], this_id;
        bool all_null = true;
        do
        {
            all_null = true;
            for (unsigned int i = 0; i < productions[fpos].right.size(); i++)
            {
                this_id = productions[fpos].right[i];
                if ((this_id < tokenAmount) && (this_id != 129) && (this_id != 128) && (this_id >= 0))
                {
                    ans.insert(this_id);
                    all_null = false;
                    break;
                }
                else
                {
                    if (id != this_id)
                        first(this_id, ans);
                    if ((this_id != 129) && (!nullable.at(this_id)))
                    {
                        all_null = false;
                        break;
                    }
                    else
                        continue;
                }
            }
            if (all_null) ans.insert(129);
            fpos++;
        }
        while ((fpos < productions.size()) && (productions[fpos].left == id));
    }

    bool LR0_core_equal(const unordered_set<mypair> &a, const unordered_set<mypair> &b)
    {
        unordered_set<mypair>::iterator it;
        if (a.size() != b.size()) return false;
        for (unordered_set< mypair >::iterator i = a.begin(); i != a.end(); i++)
        {
            it = b.find(*i);
            if (it == b.end()) return false;
        }
        return true;
    }
    
    int LR0_core_index_item(const unordered_set<mypair> &a, const vector<unordered_set<mypair> > &core_item_map)
    {
        for (unsigned int i = 0; i < core_item_map.size(); i++)
            if (LR0_core_equal(a, core_item_map[i]))
                return i;
        return -1;
    }
};

int LR_DFA::total_action = 0;
bool LR_DFA::init_flag = false;
int LR_DFA::ids = 1;

#endif