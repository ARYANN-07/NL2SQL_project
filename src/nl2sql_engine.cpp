#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

#include <mysql.h> // This is the C API header

#include <memory> // For std::unique_ptr
using std::string;
using std::vector;
using std::unordered_map;

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
            "the", "a", "an", "but", "on", "at", "for", 
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
        initializeSchema(); // This will now connect to MySQL
    }

    void initializeSchema()
{
    // This object is the connection handler
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        cout << "# ERROR: mysql_init() failed." << endl;
        return;
    }

    // --- IMP DETAILS TO CONNECT DATABASE ---
    string host = "127.0.0.1";
    string user = "root";     // e.g., "root"
    string pass = "root"; // e.g., "password"
    string dbName = "electricity";   // e.g., "electricity_billing"
    int port = 3306;
    

    // Try to connect
    if (mysql_real_connect(con, host.c_str(), user.c_str(), pass.c_str(), dbName.c_str(), port, NULL, 0) == NULL) {
        cout << "# ERROR: Could not connect to database." << endl;
        cout << "# " << mysql_error(con) << endl;
        mysql_close(con);
        return;
    }

    // This is the same SQL query as before
    string query = "SELECT table_name, column_name FROM INFORMATION_SCHEMA.COLUMNS "
                   "WHERE table_schema = '" + dbName + "'";

    // Send the query
    if (mysql_query(con, query.c_str())) {
        cout << "# ERROR: Query failed." << endl;
        cout << "# " << mysql_error(con) << endl;
        mysql_close(con);
        return;
    }

    // Store the results
    MYSQL_RES *result = mysql_store_result(con);
    if (result == NULL) {
        cout << "# ERROR: mysql_store_result() failed." << endl;
        mysql_close(con);
        return;
    }

    cout << "Reading schema from database '" << dbName << "'..." << endl;
    
    MYSQL_ROW row;
    // Loop through the results row by row
    while ((row = mysql_fetch_row(result))) {
        string tableName = row[0] ? row[0] : "NULL";
        string columnName = row[1] ? row[1] : "NULL";

        // If this is the first time we see this table, create an entry for it
        if (tables.find(tableName) == tables.end()) {
            TableInfo newTable;
            newTable.name = tableName;
            tables[tableName] = newTable;
        }
        
        // Add the column to its table's list
        tables[tableName].columns.push_back(columnName);
    }

    cout << "Schema loaded successfully." << endl;

    // Clean up
    mysql_free_result(result);
    mysql_close(con);
}

    bool tableExists(string n) {
        return tables.find(n) != tables.end();
    }
    
    bool columnExistsInTable(string col, string table) {
        if (!tableExists(table)) {
            return false;
        }
        for (const string& c : tables[table].columns) {
            if (c == col) {
                return true;
            }
        }
        return false;
    }

    vector<string> getTableNames() {
        vector<string> names;
        for(auto const& pair : tables) {
            names.push_back(pair.first);
        }
        return names;
    }
};

struct QueryComponent
{
    string select="select";
    vector<string> select_fields;
    vector<string> from_tables;
    vector<string> where_conditions;
};

struct ValidationResult {
    QueryComponent components;
};

class PatternMatcher
{
private:
    // This holds the live schema from main
    DatabaseSchema& schema;

    map<string, string> tableSynonyms = {
        {"customer", "customer"},
        {"customers", "customer"},
        {"user", "customer"},
        {"users", "customer"},
        {"account", "account"},
        {"accounts", "account"},
        {"billing", "billing"},
        {"bill", "billing"},
        {"bills", "billing"},
        {"admin", "admin"},
        {"admins", "admin"},
        {"electric", "elecboard"},
        {"board", "elecboard"},
        {"eboard", "elecboard"},
        {"electricity", "elecboard"},
        {"invoice", "invoice"},
        {"invoices", "invoice"},
        {"tariff", "tariff"},
        {"tariffs", "tariff"},
        {"rate", "tariff"}
    };
    
    map<string, string> columnSynonyms = {
        {"name", "custName"},
        {"names", "custName"},
        {"id", "custID"},
        {"customerid", "custID"},
        {"address", "address"},
        {"city", "city"},
        {"pincode", "pincode"},
        {"pin", "pincode"},
        {"zip", "pincode"},
        {"accno", "accNO"},
        {"meter", "meterLine"},
        {"meterno", "meterLine"},
        {"amount", "amount"},
        {"cost", "amount"},
        {"bill", "amount"},
        {"units", "unitsConsumed"},
        {"consumption", "unitsConsumed"},
        {"rate", "rate_per_unit"},
        {"adminname", "adminName"},
        {"tarifftype", "tariffType"},
        {"boardname", "eboardName"}
    };

public:
    // This constructor saves the live schema
    PatternMatcher(DatabaseSchema& dbSchema) : schema(dbSchema) {}

    QueryComponent extractComponent(const vector<string>& tokens, vector<string>& warnings)
    {
        QueryComponent components;
        
        components.from_tables = extractTables(tokens); 
        components.select_fields = extractColumns(tokens); 
        components.where_conditions = extractConditions(tokens, columnSynonyms, warnings);

        return components;
    }

private:
    vector<string> extractTables(const vector<string>& tokens)
    {
        vector<string> tables;
        for (const string& token : tokens)
        {
            // Check if the token is a known synonym
            auto it = tableSynonyms.find(token);
            if (it != tableSynonyms.end())
            {
                // Get the "real" table name (e.g., "customer")
                string realTable = it->second;

                //    Checkinf if this table *actually exists* in our live schema
                if (schema.tableExists(realTable)) 
                {
                    // Add it only if we haven't already
                    bool found = false;
                    for(const string& t : tables) {
                        if (t == realTable) found = true;
                    }
                    if (!found) {
                        tables.push_back(realTable);
                    }
                }
            }
        }
        if(tables.empty()) tables.push_back("customer"); //by default customer table
        return tables;
    }

vector<string> extractColumns(const vector<string>& tokens)
{
    vector<string> columns;

    // if user says "everything" or "details"
    for (const string &token : tokens) {
        if (token == "everything" || token == "details") {
            return { "*" };
        }
    }

    // go through all tokens and check synonyms
    for (const string &token : tokens)
    {
        auto it = columnSynonyms.find(token);
        if (it != columnSynonyms.end())
        {
            string realColumn = it->second;

            // check if this column actually exists in schema
            bool exists = false;
            for (const string &tbl : schema.getTableNames()) {
                if (schema.columnExistsInTable(realColumn, tbl)) {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                continue; // skip if column doesn't exist

            // avoid duplicates
            bool found = false;
            for (const string &c : columns) {
                if (c == realColumn) {
                    found = true;
                    break;
                }
            }

            if (!found)
                columns.push_back(realColumn);
        }
    }

    if (columns.empty())
        columns.push_back("*");

    return columns;
}

// Helper: true if token is a known table name or a table synonym
bool isTableToken(const string &tok) {
    // check exact table name
    if (schema.tableExists(tok)) return true;

    // check synonyms map (tableSynonyms is your map<string,string>)
    auto it = tableSynonyms.find(tok);
    if (it != tableSynonyms.end()) return true;

    // normalized check can be added later, keep minimal for now
    return false;
}

    // Helper: check if a token looks like a number (allow commas and decimals)
bool isNumberToken(const string &s) {
    if (s.empty()) return false;
    int digits = 0;
    for (char c : s) {
        if (std::isdigit((unsigned char)c)) digits++;
        else if (c == '.' || c == ',') continue;
        else return false;
    }
    return digits > 0;
}

vector<string> extractConditions(const vector<string>& tokens,
                                 const map<string,string>& columnSynonyms,
                                 vector<string>& warnings)
{
    vector<string> conditions;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const string &tok = tokens[i];

        
        // handle "from state X" or "in state X" -> state = 'X'
        if ((tok == "from" || tok == "in") && i + 2 < tokens.size()) {
        if (tokens[i+1] == "state") {
        string value = tokens[i+2];
        conditions.push_back("state = '" + value + "'");
        i += 2; // consumed 'state' and the state-name
        continue;
        }
    }


        // 2) "from X" or "in X" -> treat as city = 'X'
        if ((tok == "from" || tok == "in") && i + 1 < tokens.size()) {
            string next = tokens[i + 1];

            // IF the next token is a table name (or synonym), treat it as table mention, NOT city
            if (isTableToken(next)) {
                ++i;
                continue;
            }

            // otherwise treat it as city
            conditions.push_back("city = '" + next + "'");
            ++i; 
            continue;
        }


        // 3) "between X and Y"
        if (tok == "between" && i + 3 < tokens.size() && tokens[i+2] == "and") {
    string a = tokens[i+1];
    string b = tokens[i+3];

    if (isNumberToken(a) && isNumberToken(b)) {
        conditions.push_back("amount BETWEEN " + a + " AND " + b);
    } else {
        conditions.push_back("amount BETWEEN '" + a + "' AND '" + b + "'");
    }
    i += 3;
    continue;
}


        // 4) amount / cost / bill comparisons
        // amount / cost / bill -> only treat as WHERE if a real comparison exists
// 4) amount / cost / bill numeric comparisons
if (tok == "amount" || tok == "cost" || tok == "bill") {

    string w1 = (i + 1 < tokens.size()) ? tokens[i+1] : "";
    string w2 = (i + 2 < tokens.size()) ? tokens[i+2] : "";
    string w3 = (i + 3 < tokens.size()) ? tokens[i+3] : "";

    bool parsed = false;

    // -----------------------------
    //  amount more than 2000
    //  amount is more than 2000
    // -----------------------------
    if ((w1 == "more" || w1 == "greater" || (w1 == "is" && w2 == "more") || (w1 == "is" && w2 == "greater")) &&
        (w2 == "than" || w3 == "than"))
    {
        string val = "";
        if (w2 == "than" && i + 3 < tokens.size() && isNumberToken(w3))
            val = w3;
        else if (w3 == "than" && i + 4 < tokens.size() && isNumberToken(tokens[i+4]))
            val = tokens[i+4];

        if (!val.empty()) {
            conditions.push_back("amount > " + val);
            parsed = true;
            i += 3;  // skip tokens used
        }
    }

    // -----------------------------
    //  amount less than 500
    //  amount is less than 500
    // -----------------------------
    else if ((w1 == "less" || (w1 == "is" && w2 == "less")) &&
             (w2 == "than" || w3 == "than"))
    {
        string val = "";
        if (w2 == "than" && i + 3 < tokens.size() && isNumberToken(w3))
            val = w3;
        else if (w3 == "than" && i + 4 < tokens.size() && isNumberToken(tokens[i+4]))
            val = tokens[i+4];

        if (!val.empty()) {
            conditions.push_back("amount < " + val);
            parsed = true;
            i += 3;
        }
    }

    // -----------------------------
    //  amount equal to 400
    //  amount is equal to 400
    //  amount equals 400
    // -----------------------------
    else if (w1 == "equals" && isNumberToken(w2)) {
        conditions.push_back("amount = " + w2);
        parsed = true;
        i += 2;
    }
    else if (w1 == "equal" && w2 == "to" && isNumberToken(w3)) {
        conditions.push_back("amount = " + w3);
        parsed = true;
        i += 3;
    }
    else if (w1 == "is" && w2 == "equal" && w3 == "to" &&
             i + 4 < tokens.size() && isNumberToken(tokens[i+4])) {
        conditions.push_back("amount = " + tokens[i+4]);
        parsed = true;
        i += 4;
    }

    // If parsed, stop processing this token
    if (parsed) continue;

    // If the user started a comparison but gave no number — warn
    bool startsComparison =
        (w1 == "more" || w1 == "less" || w1 == "equals" || w1 == "equal" ||
         w1 == "is" || w1 == ">" || w1 == "<" || w1 == ">=" || w1 == "<=");

    if (startsComparison) {
        warnings.push_back("Could not parse a numeric value after 'amount'.");
    }

//    parsed and NOT a comparison attempt → treat as SELECT column.
    continue;
}


        // 5) contains / has / includes -> previous token is field name (e.g., "name contains john")
        if ((tok == "contains" || tok == "has" || tok == "includes")) {
    if (i > 0 && i + 1 < tokens.size()) {
        string columnWord = tokens[i - 1];   // treat previous token as column name
        string value = tokens[i + 1];        // value after 'contains'

        // raw condition (mapping happens later in mapWhereUsingSynonyms)
        conditions.push_back(columnWord + " LIKE '%" + value + "%'");

        ++i; // skip value token
        continue;
    } else {
        warnings.push_back("Found 'contains' but could not detect column or value.");
        continue;
    }
}

        // 6) "field in a b c" -> if token is a field name and next tokens look like values, collect some values
        //    e.g., "city in pune mumbai" (we accept upto 3 tokens for simplicity)

        // ignore pattern "in state X"
        if (i + 2 < tokens.size()) {
        if (tokens[i+1] == "in" && tokens[i+2] == "state") continue;  }

        //Actual implementation of field in a b c
    if (i + 2 < tokens.size() && tokens[i + 1] == "in") {

    string columnWord = tok;  // treat current token as column

    vector<string> vals;
    size_t j = i + 2;

    // collect up to 3 values (simple)
    for (; j < tokens.size() && vals.size() < 3; ++j) {
        string t = tokens[j];
        if (t == "and" || t == "who" || t == "with" || t == "where")
            break;
        vals.push_back(t);
    }

    if (!vals.empty()) {
        string clause = columnWord + " IN (";
        for (size_t k = 0; k < vals.size(); ++k) {
            if (k) clause += ", ";
            clause += "'" + vals[k] + "'";
        }
        clause += ")";
        conditions.push_back(clause);

        i = j - 1; // advance index
        continue;
    }
}

    } // end loop tokens

    return conditions;
}


};
// Helper function: makes sure that all cols are from same table
bool areColumnsValid(QueryComponent& components, DatabaseSchema &schema, vector<string> &warnings){
    string tbl = components.from_tables[0];  // NOTE that table component is never empty(default customer)

    // if SELECT *
    if (components.select_fields.size() == 1 && components.select_fields[0] == "*") {
        return true;
    }

    bool valid = true;

    for (const string &col : components.select_fields) {
        if (!schema.columnExistsInTable(col, tbl)) {
            warnings.push_back("Column '" + col + "' does not exist in table '" + tbl + "'.");
            valid = false;
        }
    }
    return valid;
}

class SQLBuilder 
{
public:
    string buildQuery(const QueryComponent& components, vector<string>& warnings) 
    {
        string query = "SELECT ";
        
       
        if (components.select_fields.empty())  // Add columns to select
        {
            query += "*";
        }
        
        else 
        {
            for (int i = 0; i < components.select_fields.size(); i++) 
            {
                if (i > 0) query += ", ";
                query += components.select_fields[i];
            }
        }
        
        
        query += " FROM ";
        if (components.from_tables.empty()) 
        {
            query += "customer";
        } 
        else 
        {
            string tableToUse = components.from_tables[0];
            query += tableToUse; 
            if (components.from_tables.size() > 1) {
                warnings.push_back("JOIN attempted. Multiple tables detected, but JOINs are disabled. Using only '" + tableToUse + "'.");
            }
        }

        // WHERE clause: append conditions if any
        if (!components.where_conditions.empty()) {
            // If whereClauses refer to columns probably not in fromTable, you should validate earlier.
            query += " WHERE ";
            for (size_t i = 0; i < components.where_conditions.size(); ++i) {
                if (i) query += " AND ";
                query += components.where_conditions[i];
            }
        }
        
        return query;
    }
};

int main()
{
    Tokenizer tokenizer;
    vector<string> warning;

    // This line creates the schema object.
    // The constructor calls initializeSchema(), which connects to the DB.
    cout << "Attempting to load schema..." << endl;
    DatabaseSchema schema; 
    cout << "Schema loading complete." << endl;

    // Pass the schema to the PatternMatcher.
    PatternMatcher matcher(schema); 

     SQLBuilder builder;

     string temp=tokenizer.input(); // This runs AFTER the DB connection

     vector<string> simplified = tokenizer.tokenize(temp);
    QueryComponent components = matcher.extractComponent(simplified, warning); 
    bool valid = areColumnsValid(components, schema, warning); //check if all columns belong to same table
     string sql = builder.buildQuery(components, warning); 

    if(!warning.empty()){
        for(int i=0; i<warning.size(); i++){
            cout << "WARNING \n" << i + 1 << ": " << warning[i] << "\n";
        }
    }
     cout << "Generated Query is: " << sql << endl;

     return 0;
}