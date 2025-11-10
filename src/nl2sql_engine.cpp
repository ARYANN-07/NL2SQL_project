#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_set>

#include <mysql.h> // This is the C API header

#include <memory> // For std::unique_ptr
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

    // --- ⚠️ YOU MUST EDIT THESE DETAILS ---
    string host = "127.0.0.1";
    string user = "root";     // e.g., "root"
    string pass = "root"; // e.g., "password"
    string dbName = "sample_electricity_billing";   // e.g., "electricity_billing"
    int port = 3306;
    // --- END OF EDIT SECTION ---

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
    // This function is new. It's needed for the "Hybrid" approach.
    bool tableExists(string n) {
        return tables.find(n) != tables.end();
    }
    
    // This function is new. It's needed for the "Hybrid" approach.
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

    // This is your old function, it's still useful
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

class PatternMatcher
{
private:
    // This holds the live schema from main
    DatabaseSchema& schema;

    // --- SYNONYM DICTIONARIES ---
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
        {"electric", "elec_board"},
        {"board", "elec_board"},
        {"electricity", "elec_board"},
        {"invoice", "invoice"},
        {"invoices", "invoice"},
        {"tariff", "tariff"},
        {"tariffs", "tariff"},
        {"rate", "tariff"}
    };
    
    map<string, string> columnSynonyms = {
        {"name", "cust_name"},
        {"names", "cust_name"},
        {"id", "cust_ID"},
        {"customerid", "cust_ID"},
        {"address", "address"},
        {"city", "city"},
        {"pincode", "pincode"},
        {"pin", "pincode"},
        {"zip", "pincode"},
        {"accno", "acc_no"},
        {"meter", "meter_no"},
        {"meterno", "meter_no"},
        {"amount", "amount"},
        {"cost", "amount"},
        {"bill", "amount"},
        {"units", "monthly_unit"},
        {"consumption", "monthly_unit"},
        {"rate", "per_unit"},
        {"adminname", "admin_name"},
        {"tarifftype", "tariff_type"},
        {"boardname", "board_name"}
    };

public:
    // This constructor saves the live schema
    PatternMatcher(DatabaseSchema& dbSchema) : schema(dbSchema) {}

    QueryComponent extractComponent(const vector<string>& tokens)
    {
        QueryComponent components;
        // These functions will now use the live schema
        components.from_tables = extractTables(tokens); 
        components.select_fields = extractColumns(tokens); 

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

                //    Check if this table *actually exists* in our live schema
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
        return tables;
    }

    vector<string> extractColumns(const vector<string>& tokens)
    {
        vector<string> columns;
        for (const string& token : tokens) {
            if (token == "everything" || token == "details") {
                columns.push_back("*");
                return columns;
            }
        }

        for (const string& token : tokens)
        {
            // Check if the token is a known synonym
            auto it = columnSynonyms.find(token);
            if (it != columnSynonyms.end())
            {
                // 2. Get the "real" column name 
                string realColumn = it->second;

                //    Here, we would ideally check if the column
                //    exists in the tables we found.
                //    For now, we just add it.
                //
                //    A better implementation would check:
                //    schema.columnExistsInTable(realColumn, "some_table")
                
                // Add it only if we haven't already
                bool found = false;
                for(const string& c : columns) {
                    if (c == realColumn) found = true;
                }
                if (!found) {
                    columns.push_back(realColumn);
                }
            }
        }

        if (columns.empty()) {
            columns.push_back("*");
        }
        return columns;
    }
};

class SQLBuilder 
{
public:
    string buildQuery(const QueryComponent& components) 
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
            query += components.from_tables[0]; 
        }
        
        return query;
    }
};

int main()
{
    Tokenizer tokenizer;

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
    QueryComponent components = matcher.extractComponent(simplified); 
     string sql = builder.buildQuery(components); 
    
     cout << "Generated Query is: " << sql << endl;

     return 0;
}