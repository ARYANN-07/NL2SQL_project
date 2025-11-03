#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_set>
using namespace std;

class Tokenizer {
public:

     string input() // User input
    {
        string var;
        cout<<"Enter query: \n";
        getline(cin,var);
        return var;
    }

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

        // Lowercase manually
        if (processedChar >= 'A' && processedChar <= 'Z') { // Added the difference between 'a' and 'A' to convert to lowercase.
            processedChar = processedChar + ('a' - 'A');
        }

        // Removed punctuation manually
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
            "the", "a", "an", "but", "in", "on", "at", "to", "for", "is", 
            "of", "with", "by", "are", "was", "were", "be", "been", "have", "has", 
            "had", "do", "does", "did", "will", "would", "shall", "should", "could",
            "me", "my", "i", "you", "what"
        };
        
        return stopWords.count(word);
    }
};

struct TableInfo  
{
    string name;
    vector<string> columns;
};

class DatabaseSchema
{
    private:
    map<string, TableInfo> tables;

    public:
    DatabaseSchema() {
        initializeSchema(); //object create hote hi saare tables initialize hojayenge
    }

    void initializeSchema()
    {
        TableInfo account;
        account.name="account";
        account.columns={"acc_ID", "cust_ID", "acc_no", "name"};
        tables["account"] = account; //adding table name and table info in tables map

        TableInfo admin;
        admin.name="admin";
        admin.columns={"admin_id", "admin_name", "cust_ID"};
        tables["admin"]= admin;

        TableInfo billing;
        billing.name="billing";
        billing.columns={"meter_no", "acc_ID", "cust_ID", "monthly_unit", "per_unit", "amount"};
        tables["billing"]= billing;
        
        TableInfo customer;
        customer.name="customer";
        customer.columns={"cust_ID", "cust_name", "address", "city", "pincode"};
        tables["customer"]=customer;

        TableInfo elec_board;
        elec_board.name="elec_board";
        elec_board.columns={"eboard_id", "board_name"};
        tables["electric_board"]=elec_board;

        TableInfo invoice;
        invoice.name="invoice";
        invoice.columns={"invoice_id", "eboard_ID", "tariff_ID", "acc_no", "meter_no", "date"};
        tables["invoice"]=invoice;

        TableInfo tariff;
        tariff.name="tariff";
        tariff.columns={"tariff_id", "tariff_type", "rate_per_unit"};
        tables["tariff"]=tariff;
    }

    vector<string> getTableNames(){
        vector<string> names;
        for(auto pair : tables){
            names.push_back(pair.first);
        }
        return names;
    }

    bool tableExists(string n){
        bool exists=false;
        for(auto pair: tables){
            if(pair.first==n) exists=true;
        }
        return exists;
    }
};


int main(){
    // Tokenizer tokenizer;
    // string temp = tokenizer.input();
    // vector<string> simplified = tokenizer.tokenize(temp);
    // for(string token : simplified){
    //     cout<<token<<endl;
    // }

    DatabaseSchema db;
    cout<<db.tableExists("accout");
    return 0;
}