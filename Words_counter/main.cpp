//
// Created by Olia on 30.05.2017.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>
#include "measuring_time.h"

using namespace std;

mutex mx;
map<string, int> all_counted;

map<string, string> read_config(string filename) {
    string line;
    ifstream myfile;
    map<string, string> mp;
    myfile.open(filename);

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            double pos = line.find("=");
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            mp[key] = value;
        }
        myfile.close();
    }
    else {
        cout << "Error with opening the file!" << endl;
    }
    return mp;
};

bool diff_func(const pair<string, int> &a, const pair<string, int> &b){
    return a.second < b.second;
}

void alph_num_order(string f1, string f2) {
    ofstream f_a;
    f_a.open(f1);
    for (auto el : all_counted) f_a << el.first << " - " << el.second << endl;
    f_a.close();

    vector<pair<string, int> > vector_of_pairs(all_counted.begin(), all_counted.end());
    sort(vector_of_pairs.begin(), vector_of_pairs.end(), diff_func);
    ofstream f_n;
    f_n.open(f2);
    for (pair<string, int> item: vector_of_pairs){
        f_n << item.first << " - " << item.second << endl;
    }
    f_n.close();

}

template <class T>
T get_param(string key, map<string, string> myMap) {
    istringstream ss(myMap[key]);
    T val;
    ss >> val;
    return val;
}
void count_words(vector<string> words, int beg, int fin){
    map<string, int> new_map;
    for (int i = beg; i < fin; ++i) {
        ++new_map[words[i]];
    }

    lock_guard<mutex> lc(mx);
    for (auto el : new_map) all_counted[el.first] += el.second;
};

void check_word(string& word) {
    string new_word = "";
    transform(word.begin(), word.end(), word.begin(), ::tolower);
    for (auto i : word) {
        if (!ispunct(i) && !isdigit(i)) {
            new_word += i;
        }
    }
    word = new_word;
}

vector<string> file_reading(string filename) {
    vector<string> words;
    string line;
    ifstream f;
    f.open(filename);
    if (f.is_open()) {
        while (f >> line) {
            check_word(line);
            words.push_back(line);
        }
        f.close();
    }
    else
        cerr << "Error with opening the file!";
    return words;
}

int main(){
    string filename;
    cout << "Please enter name of configuration file with extension '.txt':>";
    cin >> filename;    // config.txt

    auto start_time = get_current_time_fenced();
    map<string, string> mp = read_config(filename);

    if (!mp.empty()) {
        string infile = get_param<string>("infile", mp);
        string out_by_a = get_param<string>("out_by_a", mp);
        string out_by_n = get_param<string>("out_by_n", mp);
        int num_thr = get_param<int>("threads", mp);
        vector<thread> threads;

        auto start_reading = get_current_time_fenced();
        vector<string> words = file_reading(infile);
        auto finish_reading = get_current_time_fenced();

        int delta = words.size() / num_thr;
        size_t beg = 0, fin = words.size() - 1;

        auto start_thr = get_current_time_fenced();
        for (int i = 0; i < num_thr - 1; ++i) {
            threads.push_back(thread(count_words, words, beg, beg + delta));
            beg += delta;
        }
        threads.push_back(thread(count_words, words, beg, fin + 1));
        for (auto &th : threads) th.join();
        auto finish_thr = get_current_time_fenced();

        all_counted.erase("");
        alph_num_order(out_by_a, out_by_n);
        auto final_time = get_current_time_fenced();

        chrono::duration<double, milli> total_time = final_time - start_time;
        chrono::duration<double, milli> reading_time = finish_reading - start_reading;
        chrono::duration<double, milli> analyzing_time = finish_thr - start_thr;

        cout << "Total: " << total_time.count() << " ms" << endl;
        cout << "Reading time: " << reading_time.count() << " ms" << endl;
        cout << "Analyzing: " << analyzing_time.count() << " ms" << endl;
    }
    return 0;
}
