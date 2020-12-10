#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include "File.h"
#include "Vocab.h"
#include "Ngram.h"

using namespace std;

#define NGRAM_ORDER 2

double get_bigram_prob(Vocab &voc, Ngram &lm, const char *w_1, const char *w_2) {
  VocabIndex w1_id = voc.getIndex(w_1);
  VocabIndex w2_id = voc.getIndex(w_2);

  if (w1_id == Vocab_None) {
    w1_id = voc.getIndex(Vocab_Unknown);
  }
  if (w2_id == Vocab_None) {
    w2_id = voc.getIndex(Vocab_Unknown);
  } 

  VocabIndex context[] = {w1_id, Vocab_None};
  return lm.wordProb(w2_id, context);
}

double getTrigramProb(Vocab &voc, Ngram &lm, const char *w_1, const char *w_2, const char *w_3) {
  VocabIndex w1_id = voc.getIndex(w_1);
  VocabIndex w2_id = voc.getIndex(w_2);
  VocabIndex w3_id = voc.getIndex(w_3);

  if (w1_id == Vocab_None) {
    w1_id = voc.getIndex(Vocab_Unknown);
  }
  if (w2_id == Vocab_None) {
    w2_id = voc.getIndex(Vocab_Unknown);
  }
  if (w3_id == Vocab_None) {
    w3_id = voc.getIndex(Vocab_Unknown);
  }

  VocabIndex context[] = {w2_id, w1_id, Vocab_None};
  return lm.wordProb( w3_id, context );
}

vector <string> seperate(string &long_str) {
  vector <string> long_str_vec;

  string delimiter = " ";
  size_t pos = 0;
  while ((pos = long_str.find(delimiter)) != std::string::npos) {
    string big5 = long_str.substr(0, pos);
    if (big5.length() == 2) {
      long_str_vec.push_back(big5);
    }
    long_str.erase(0, pos+delimiter.length());
  }
  long_str_vec.push_back(long_str);

  return long_str_vec;
}

vector < vector <int> > forward(Vocab &voc, Ngram &lm, map <string, string> &z_b_map, vector <string> &sent) {
  // Score and path for a sequence (one line).
  vector < vector <double> > score_vec (sent.size());
  vector < vector <int> > path_vec (sent.size());

  for (int j = 1; j < sent.size(); j++) {
    string prev_big5_string = z_b_map[sent[j - 1]];
    string curr_big5_string = z_b_map[sent[j]];

    if (j == 1) {
      prev_big5_string = "<s>";
    }
    if (j == sent.size() - 1) {
      curr_big5_string = "</s>";
    }
    if (curr_big5_string.length() < 2) {
      curr_big5_string = sent[j];
    }        

    vector <string> prev_big5_vec;
    prev_big5_vec = seperate(prev_big5_string);

    vector <string> curr_big5_vec;
    curr_big5_vec = seperate(curr_big5_string);

    if (j == 1){
      for (int k = 0; k < curr_big5_vec.size(); k++){
        score_vec[j].push_back(get_bigram_prob(voc, lm, "<s>", curr_big5_vec[k].c_str()));
        path_vec[j].push_back(0);
      }
    }
    else{
      for (int k = 0; k < curr_big5_vec.size(); k++) {
        double max_prob = -std::numeric_limits<double>::max();
        int max_prob_path = 0;
        for (int l = 0; l < prev_big5_vec.size(); l++) {
          double prob = score_vec[j - 1][l] + get_bigram_prob(voc, lm, prev_big5_vec[l].c_str(), curr_big5_vec[k].c_str());
          if (prob > max_prob && prob < 0) {
            max_prob = prob;
            max_prob_path = l;
          }
        }
        score_vec[j].push_back(max_prob);
        path_vec[j].push_back(max_prob_path);
      }
    }
  }

  return path_vec;
}

vector <string> backward(Vocab &voc, Ngram &lm, map <string, string> &z_b_map, vector < vector <int> > &path_vec, vector <string> &sent) {
  int max_prob_index = 0;
  vector<string> ans_vec;

  for (int j = sent.size() - 1; j >= 1; j--){
    string curr_big5_string = z_b_map[sent[j]]; 

    if (j == sent.size() - 1) {
      curr_big5_string = "</s>";
    }

    vector <string> curr_big5_vec;
    curr_big5_vec = seperate(curr_big5_string);
    
    ans_vec.push_back(curr_big5_vec[max_prob_index]);

    max_prob_index = path_vec[j][max_prob_index];
  }
  ans_vec.push_back("<s>");

  return ans_vec;
}

vector <string> Viterbi(Vocab &voc, Ngram &lm, map <string, string> &z_b_map, vector <string> &sent) {

  // Forward path
  vector < vector <int> > path_vec;
  path_vec = forward(voc, lm, z_b_map, sent);

  // Backward path
  vector <string> ans_vec;
  ans_vec = backward(voc, lm, z_b_map, path_vec, sent);

  return ans_vec;
}

int main(int argc, char *argv[]) {
  // Create ngram vocabulary
  Vocab ngram_voc;
  // Create mgram language model
  Ngram ngram_lm(ngram_voc, NGRAM_ORDER);

  // Read language model file
  File lm_file(argv[3], "r");
  ngram_lm.read(lm_file);
  lm_file.close();

  // Temporary storage
  string line;

  // Read mapping file
  ifstream map_file(argv[2]);
  map <string, string> zhuyin_big5_map;
  while (getline(map_file, line)) {
    size_t pos = line.find(" ");

    string key = line.substr(0, pos);
    string values = line.erase(0,pos + 1);

    zhuyin_big5_map[key] = values;
  }
  map_file.close();

  // Read text file
  ifstream test_file(argv[1]);
  vector < vector <string> > test_vec;
  while (getline(test_file, line)) {
    vector <string> temp_vec;

    temp_vec.push_back("<s>");
    
    string delimiter = " ";
    size_t pos = 0;
    while ((pos = line.find(delimiter)) != std::string::npos) {
      string big5 = line.substr(0, pos);
      if (big5.length() == 2) {
        temp_vec.push_back(big5);
      }
      line.erase(0, pos + delimiter.length());
    }

    temp_vec.push_back("</s>");  
    test_vec.push_back(temp_vec);
  }
  test_file.close();

  // Open output file
  ofstream ans_file(argv[4]);

  // Viterbi algorithm
  for (int i = 0; i < test_vec.size(); i++){
    vector <string> temp_vec;
    temp_vec = Viterbi(ngram_voc, ngram_lm, zhuyin_big5_map, test_vec[i]);

    // Output sequence from back to front.
    for (int j = temp_vec.size() - 1; j >= 1; j--){
      if (temp_vec[j] != "") {
        ans_file << temp_vec[j] << " ";
      }
      else {
        ans_file << "<unk> ";
      }
    }
    ans_file << temp_vec[0] << endl;       
  }
  return 0;
}
