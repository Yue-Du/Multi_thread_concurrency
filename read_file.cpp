#include<stdio.h>
#include<string.h>
#include  <map>
#include <iostream>
#include <list>
#include <algorithm>

using namespace std;

int compare_str(string a, string b){
    for(int i=0; i<min(a.size(), b.size()); i++){
        if(a[i] > b[i]){
            return 1;
        }else if(a[i] < b[i]){
            return -1;
        }
    }
    if(a.size()==b.size()){
        return 0;
    }else if(a.size() < b.size()){
        return 1;
    }else{
        return -1;
    }
}

bool ReadTxt(char filename[]) {
    map<string, list<int> > word_dict;
    char data[100];
    int file_num =-1;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("can't open file\n");
        return false;
    }

    while (!feof(fp)) {
        fgets(data, sizeof(data), fp);
        char content[100];
        if (data[strlen(data) - 1] == '\n') {
            data[strlen(data) - 1] = '\0';
        }

        FILE *ff = fopen(data, "r");
        if (!ff) {
            return false;
        }
        file_num +=1;
        while (!feof(ff)) {
            fgets(content, sizeof(content), ff);
            string s(content);
            string sub;
            int ascii;
            int begin_ind = 0;
            for (int i = 0; i < s.size(); i++) {
                ascii = (int) s.at(i);
                if (ascii < 65 or ascii > 122 or (ascii > 90 and ascii < 97)) {
                    if (i == begin_ind) {
                        begin_ind = i + 1;
                        continue;
                    }
                    sub = s.substr(begin_ind, i - begin_ind);
                    begin_ind = i + 1;
                    if(word_dict.find(sub) == word_dict.end()){
                        word_dict[sub].push_back(file_num);
                    }
                    else if(word_dict[sub].back() != file_num){
                        word_dict[sub].push_back(file_num);
                    }
                }
                if((i ==s.size()-1) and i>=begin_ind){
                    sub = s.substr(begin_ind, i - begin_ind+1);
                    if(word_dict.find(sub) == word_dict.end()){
                        word_dict[sub].push_back(file_num);
                    }
                    else if(word_dict[sub].back() != file_num){
                        word_dict[sub].push_back(file_num);
                    }
                }
            }
            content[0] = '\0';
        }
        fclose(ff);
    }

    map< string,list<int> >::iterator iter;
    list<string> alpha;
    for(iter = word_dict.begin(); iter != word_dict.end(); iter++){
        alpha.push_back((iter->first));
    }
    alpha.sort(compare_str);
    list<string>::iterator itlist;
    for(itlist = alpha.begin(); itlist != alpha.end(); itlist++){
        cout<<*itlist<<':';
        list<int>::iterator it;
        list<int> lst = word_dict[*itlist];
        for (it = lst.begin(); it != lst.end(); it++){
            cout<<' '<<*it;
        }
        printf("\n");
    }
    printf("\n");
    fclose(fp);
    return true;
}

int main(int argc, char * argv[]) {
    printf("argc: %d \n", argc);
    for(int i=0; i<argc; i++){
        printf("argv %d: %s \n", i, argv[i]);
    }
    //ReadTxt((argv[1]));
    ReadTxt("../yue.txt");
    return 0;
}//
// Created by 杜悦 on 2020/6/29.
//

