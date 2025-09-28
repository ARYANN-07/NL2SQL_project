#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
using namespace std;

class NLTokenizer {
public:
    vector<string> tokenize(const string& input) {
        vector<string> tokens;
        
        // Used a stringstream to easily split the input by spaces.(can also be done by filtering each char and splitting on space but lengthy)
        stringstream ss(input);
        string token;

        while (ss >> token) { 
            // Process each token individually.
            string processedToken = processToken(token);
            
            // Add the token only if it's not empty and not a stop word.
            if (!processedToken.empty() && !isStopWord(processedToken)) {
                tokens.push_back(processedToken);
            }
        }
        
        return tokens;
    }

private:
    string processToken(string token) {
        string result; 

    for (char c : token) {
        char processedChar = c;

        // Lowercase 
        if (processedChar >= 'A' && processedChar <= 'Z') { // Added the difference between 'a' and 'A' to convert to lowercase.
            processedChar = processedChar + ('a' - 'A');
        }

        // Remove punctuation 
        if ((processedChar >= 'a' && processedChar <= 'z') || (processedChar >= '0' && processedChar <= '9')) {
            result += processedChar; 
        }
    }
    
    return result;
    }
    
    //Filtering the useless words
    bool isStopWord(const string& word) {
        // It's static so it's only created once for the entire class.
        static const unordered_set<string> stopWords = {
            "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "is", 
            "of", "with", "by", "are", "was", "were", "be", "been", "have", "has", 
            "had", "do", "does", "did", "will", "would", "shall", "should", "could",
            "me", "my", "i", "you", "what"
        };
        
        return stopWords.count(word);
    }
};

int main(){
    NLTokenizer tokenizer;
    vector<string> simplified = tokenizer.tokenize("Hi, mY name is 'ARYAN JAGTAP' , glad  to meet you!!");
    for(const auto token : simplified){
        cout<<token<<endl;
    }

    return 0;
}