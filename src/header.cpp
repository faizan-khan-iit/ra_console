#include "header.h"

// Name of all tables as defined in db_info.txt
// Any other table names will not be considered
vector<string> all_tables_in_database;

// Temprorarily store renamed table names
vector<string> currently_renamed_tables;
string curr_db = "";


//////////////////////
// Helper Functions //
//////////////////////


/// Initialize program and determine the database to use
/// Input: None
/// Output: None
string start_up(){
    cout << "Starting terminal...";
    // Get valid database name
    curr_db = get_database();
    newline();
    cout << "Database changed";
    newline();
    return curr_db;
}


/// Check if db_info.txt of database exists and is a database file
/// Input: Database name to search folder and db_info.txt file
/// Output: TRUE if database is present, FALSE otherwise
bool check_if_database_folder(string db_name){
    string path = string("./") + db_name + string("/db_info.txt");
    ifstream db_info_file(path);
    string one_line;
    // If file is present in given format, the folder is a database folder
    if(db_info_file.is_open()){
        getline(db_info_file,one_line);
        db_info_file.close();
        if(one_line == string("## database ##")){
            return(true);
        }
    }
    // Otherwise it is not a database folder
    return(false);
}


/// Read the db_info.txt file and prints it
/// Input: Database name from which to read db_info.txt file
/// Output: None
void get_db_info(string db_name){
    string path = string("./") + db_name + string("/db_info.txt");
    ifstream db_info_file(path);
    std::cin.ignore(32767, '\n'); // ignore up to 32767 characters until a \n is removed
    string one_line;
    if(db_info_file.is_open()){
        // Clear any entries from table names of current database and load new ones
        all_tables_in_database.clear();
        while (getline(db_info_file,one_line)){
            cout << one_line << endl;
            if(one_line.substr(0,1) == string("$")){
                add_to_all_tables_vector(one_line.substr(1));
            }
        }
        db_info_file.close();
    }
}


/// Get the database name if db_info.txt is present and print it
/// Input: None
/// Output: Database name
string get_database(){
    string db_name;
    newline();
    cout << "Enter name of database: " << endl;

    bool is_database = false;
    // Keep getting input until we get a valid database folder name
    while(!is_database){
        cin >> db_name;
        is_database = check_if_database_folder(db_name);
        if(!is_database){
            newline();
            cout << "Error:" << db_name;
            cout << " is not a correct database name. Please try again";
            cout << endl;
        }
    }
    // Print database information from db_info file
    get_db_info(db_name);
    return(db_name);
}


/// Keep getting input until "EXIT" and send to parser if valid query
/// Input: None
/// Output: 0 if successful
int get_input(){
    string result = get_whole_query();
    if(result == "EXIT"){
        return EXIT_PROGRAM;
    }
    // If no error is thrown, print result
    try{
        // If query does not have valid parenthesis, throw error
        if(!valid_par(result)){
            throw_error(1, "invalid query. Please check parenthesis");
        }
        // Clear any previously renamed table names before new query
        currently_renamed_tables.clear();

        // Store the answer for printing
        vector<string> ans = query_parser(result);
        if(ans.size() == 3){
            // The answer only has table name, col names and their types
            cout << endl << "Empty Set" << endl;
        }else if(ans.size()>3){
            // Check for invalid data
            verify_table_contents(ans);
            vector<string> one_line;
            for(int i=1;i<ans.size();i++){
                one_line.clear();
                // Skip printing column data types
                if(i==2) continue;
                one_line = split_string_by_char(ans[i], " ");
                for(auto elem : one_line){
                    printElement(elem, 15);
                }
                cout << endl;
            }
            cout << endl;
        }
    }
    // Prints any errors present
    catch(Error_info err){
        cout << endl << "ERROR: " << err.info << endl;
    }
    return 0;
}


/// Throw error and return to get_input() for error printing
/// Input: Error code, Error message to print on console
/// Output: None
void throw_error(int code, string info){
    Error_info err;
    err.code = code;
    err.info = info;
    throw err;
}


/// Return the query delimited by ';' without whitespaces
/// Input: None
/// Output: Query without whitespaces
string get_whole_query(){
    string query="";
    char c;
    while (cin >> c){
        // Anything after ';' is ingnored
        if(c == ';'){
            break;
        }
        query.push_back(c);
    }
    return(query);
}


/// Parse the query and return an appropriate table
/// Input: query to parse
/// Output: a table for the given query
vector<string> query_parser(string query){
    vector<string> ans;
    if(query == ""){
        return ans;
    }else if(query.substr(0, 1) == string("(")){
        // For nested parentheses e.g. PRO{id}(((student)))
        return(query_parser(query.substr(1)));
    }else if(query == "SHOW_TABLES"){
        show_tables();
        return ans;
    }
    string command="";
    if(query.length() > 3){
        command = query.substr(0, 4);
    }
    if(command == "SEL{"){
        ans = select_ra(query.substr(4));
    }else if(command == "PRO{"){
        ans = project_ra(query.substr(4));
    }else if(command == "UNI("){
        ans = union_ra(query.substr(4));
    }else if(command == "DIF("){
        ans = set_diff_ra(query.substr(4));
    }else if(command == "CRP("){
        ans = cart_prod_ra(query.substr(4));
    }else if(command == "REN{"){
        ans = rename_ra(query.substr(4));
    }else{
        // If no command is given, it may be a table name
        // If query has valid table name, return table
        int ctr = 0;
        bool invalid_table_name = false;
        while(ctr < query.length() &&
            query.substr(ctr, 1) != ")" &&
            query.substr(ctr, 1) != ","){
            // SEL table names are delimited by ')'
            // UNI table names are delimited by ','
            if(not_an_alphanumeric(query[ctr])){
                invalid_table_name = true;
                break;
            }
            ctr++;
        }
        if(invalid_table_name || ctr >= query.length()){
            throw_error(2, "invalid query");
        }
        bool is_table = false;

        string possible_table = query.substr(0, ctr);
        // If table_name is present in all tables name from db_info file
        for(auto table_name: all_tables_in_database){
            if(table_name == possible_table){
                ans = get_table_contents(possible_table);
                is_table = true;
                break;
            }
        }
        // The query is invalid
        if(!is_table){
            throw_error(3, "invalid query");
        }
    }
    return ans;
}


/// Decide which comma to use for splitting in UNI, DIF, CRP
/// Input: query with possible commas
/// Output: index of comma to be used for splitting
int where_to_split(string query){
    // For nested queries we need the comma at the bottom most level
    int split_here=0;
    int open_br=0, close_br=0;
    for(int j=0; j<query.size(); j++){
        if(query[j] == ',' && open_br==close_br){
            split_here = j;
            break;
        }else if(query[j] == '('){
            open_br++;
        }else if(query[j] == ')'){
            close_br++;
        }
    }
    return(split_here);
}


/// Remove table name from attribute names, e.g. converts student.id to id
/// Input: attribute name
/// Output: attribute name without table name
string remove_table_name(string entry){
    for(int i=0;i<=entry.size();i++){
        if(entry.substr(i, 1) == string(".")){
            // Ignore 'student.' from 'student.id'
            return(entry.substr(i+1));
        }
    }
    return(entry);
}


/// Return a vector of a string split by a char
/// Input: string to be split, a character to split the string by
/// Output: a vector of split substrings
vector<string> split_string_by_char(string str, string chr){
    vector<string> result;
    int last_char = -1;
    for(int i=0; i<str.length(); i++){
        if(str.substr(i, 1) == chr){
            result.push_back(str.substr(last_char+1, i-last_char-1));
            last_char = i;
        }
    }
    // Insert last element
    result.push_back(str.substr(last_char+1, str.length()-last_char-1));
    return(result);
}


/// Return all column names for a table id, name etc
/// Input: a string containing column names separated by " "
/// Output: a vector of column names
vector<string> get_cols(string col_line){
    // column names are separated by ' '
    return(split_string_by_char(col_line, " "));
}


/// Return all column names for a table, including id, student.id etc
/// Input: string containing column names, the table name to be added
/// Output: a vector of all column names
vector<string> get_all_cols(string col_line, string table_name){
    vector<string> result = split_string_by_char(col_line, " ");
    int tb_size = result.size();
    // For every col_name -> table_name.col_name
    for(int k=0;k<tb_size;k++){
        result.push_back(table_name + string(".") + result[k]);
    }
    return(result);
}


/// Return TRUE if both tables have same col names and types, used in DIF, UNI
/// Input: a pair of column names and their respective types
/// Output: TRUE if the pair is identical, FALSE otherwise
bool all_same_cols(vector<string> table_cols1, vector<string> table_cols2,
    vector<string> col_types1, vector<string> col_types2){
    if(table_cols1.size() != table_cols2.size()){
        return(false);
    }
    // All col1 names in col2
    for(auto col1 : table_cols1){
        if(find(table_cols2.begin(), table_cols2.end(), col1) == table_cols2.end()){
            return(false);
        }
    }
    // All col2 names in col1
    for(auto col2 : table_cols2){
        if(find(table_cols1.begin(), table_cols1.end(), col2) == table_cols1.end()){
            return(false);
        }
    }
    // Identical column types
    for(int i=0;i<table_cols1.size();i++){
        for(int j=0;j<table_cols2.size();j++){
            if(table_cols1[i] == table_cols2[j]){
                if(col_types1[i]!=col_types2[j]){
                    return(false);
                }
            }
        }
    }
    return(true);
}


/// Return TRUE if any two col names are same, used in CRP
/// Input: a pair of column names
/// Output: TRUE if any two column names are same, FALSE otherwise
bool any_col_names_same(vector<string>table_cols1, vector<string>table_cols2){
    for(auto col1 : table_cols1){
        for(auto col2 : table_cols2){
            if(col1 == col2){
                return(true);
            }
        }
    }
    return(false);
}


/// Return true if any column name is repeated in a table
/// Input: column names from a single table
/// Output: TRUE if no column name is repeated in table
bool repeated_columns(vector<string> col_names){
    for(int i=0; i<col_names.size(); i++){
        for(int j=0; j<col_names.size(); j++){
            if(col_names[i] == col_names[j] && i!=j){
                return true;
            }
        }
    }
    return false;
}


/// Return the order of columns to be used in Union/Set Diff
/// Input: a pair of column names
/// Output: a vector of ints to decide the order of attributes
vector<int> get_order(vector<string> t_cols1, vector<string> t_cols2){
    vector<int> result;
    for(int i=0;i<t_cols1.size();i++){
        for(int j=0;j<t_cols2.size();j++){
            if(t_cols1[i] == t_cols2[j]){
                result.push_back(j);
            }
        }
    }
    return(result);
}


/// Returns a vector with first element as New table name, followed by attributes, if any
/// Input: a predicate to RENAME
/// Output: a vector of strings containing new table details
vector<string> get_new_table_info(string predicate){
    vector<string> result;
    size_t found = predicate.find("|");
    string tb_name;
    if (found!=std::string::npos){
        tb_name = predicate.substr(0, found);
    }else{
        // We only have table name, no column names given in RENAME
        result.push_back(predicate);
        return(result);
    }
    result.push_back(tb_name);
    // Get column names
    vector<string> some_vec = split_string_by_char(predicate.substr(found+1), ":");
    for(auto i : some_vec){
        result.push_back(i);
    }
    return(result);
}


/// Return the table contents as a vector of strings
/// Input: table name to get text file
/// Output: table data stored in a vector of strings
vector<string> get_table_contents(string table_name){
    vector<string> table;
    // Read from file
    string path = string("./") + curr_db + string("/tables/") +
    table_name + string(".txt");
    ifstream table_file(path);
    string one_line;
    if(table_file.is_open()){

        while (getline(table_file,one_line)){
            table.push_back(one_line);
        }
        table_file.close();
    }else{
        // Table is in db_info but no file present
        throw_error(18, string("File for table '") + table_name + string("' not found"));
    }
    verify_table_contents(table);
    return(table);
}


/// Checks if parentheses are balanced or not
/// Input: a string to check validity of parentheses
/// Output: TRUE if parentheses are balanced, FALSE otherwise
bool valid_par(string exp){
    stack<char> S;
    for(auto i:exp){
        if((i=='{') || (i=='[') || (i=='(')){
            S.push(i);
        }
        else if((i=='}')||(i==']')||(i==')')){
            if(S.empty()){
                return false;
            }
            else if((i=='}') && (S.top() != '{')){
                return false;
            }
            else if((i==']') && (S.top() != '[')){
                return false;
            }
            else if((i==')') && (S.top() != '(')){
                return false;
            }
            else{
                S.pop();
            }
        }
    }
    if(S.empty()){
        return true;
    }
    else{
        return false;
    }
}


/// Adds table name to all_tables_in_database vector
/// Input: table name from db_info.txt file
/// Output: None
void add_to_all_tables_vector(string table_name){
    all_tables_in_database.push_back(table_name);
}


/// Display names of all tables as mentioned in db_info.txt
/// Input: None
/// Output: None
void show_tables(){
    cout << "Tables in database: " << endl;
    for(int k=0; k<all_tables_in_database.size(); k++){
        cout << endl  << k+1 << ": "<< all_tables_in_database[k];
    }
    cout << endl;
}


/// Return TRUE if character is not an alphanumeric
/// Input: character to determine
/// Output: TRUE if character is not an alphanumeric, FALSE otherwise
bool not_an_alphanumeric(char c){
    return(!isalnum(c));
}


//////////////////////////////////////////////////////////////////////////////////////////
////////////////// SELECT HELPERS ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


/// Convert an infix expression to postfix for SELECT
/// Input: expression in infix form
/// Output: expression in postfix form
string convert_to_postfix(string expression){
	stack<char> S;
	string postfix = "";
	for(int i = 0;i< expression.length();i++) {
		if(expression[i] == ' ' || expression[i] == ',') continue;
		else if(IsOperator(expression[i])){
			while(!S.empty() && S.top() != '(' && has_higher_precedence(S.top(),expression[i])){
				postfix+= S.top();
				S.pop();
			}
			S.push(expression[i]);
		}
		else if(IsOperand(expression[i])){
			postfix +=expression[i];
		}
		else if (expression[i] == '('){
			S.push(expression[i]);
		}
		else if(expression[i] == ')'){
			while(!S.empty() && S.top() !=  '(') {
				postfix += S.top();
				S.pop();
			}
			S.pop();
		}
	}
	while(!S.empty()){
		postfix += S.top();
		S.pop();
	}
	return postfix;
}


/// Verify whether a character is english letter or numeric digit or ' or .
/// Input: character to classify
/// Output: TRUE if alphanumeric or ' or .
bool IsOperand(char C){
	if(C >= '0' && C <= '9') return true;
	if(C >= 'a' && C <= 'z') return true;
	if(C >= 'A' && C <= 'Z') return true;
	if(C == '\'' || C == '.') return true;
	return false;
}


/// Verify whether a character is operator symbol or not
/// Input: character to classify
/// Output: TRUE if an operator, FALSE otherwise
bool IsOperator(char C){
	if(C == '+' || C == '-' || C == '*' || C == '/' || C== '&' || C== '|' || C== '=' ||
	 C== '>' || C== '<' || C== '!')
		return true;
	return false;
}


/// Get weight of an operator
/// Input: a character representing an operator
/// Output: weight of operator
int get_operator_weight(char op){
	int weight = -1;
	switch(op)
	{
	case '|':
        weight = 0;
        break;
    case '&':
        weight = 1;
        break;
    case '=':
    case '!':
    case '>':
    case '<':
        weight = 2;
        break;
	case '+':
	case '-':
		weight = 3;
		break;
	case '*':
	case '/':
		weight = 4;
		break;
	}
	return weight;
}


/// Determine which operator has higher precedence
/// Input: a pair of operators
/// Output: TRUE if 1st operator has higher precedence, FALSE otherwise
int has_higher_precedence(char op1, char op2){
	int op1Weight = get_operator_weight(op1);
	int op2Weight = get_operator_weight(op2);
	return op1Weight > op2Weight ?  true: false;
}


/// Return where to split a predicate
/// Input: a predicate, the operators by which to split the predicate
/// Output: a vector of split points
vector<int> get_split_points(string predicate, vector<char> valid_splits){
    vector<int> split_points = {-1};
    for(int i=0; i<predicate.size(); i++){
        // Get every occurence of separators
        if(find(valid_splits.begin(), valid_splits.end(), predicate[i]) != valid_splits.end()){
            split_points.push_back(i);
        }
    }
    return(split_points);
}


/// Get a vector of all operands in the predicate
/// Input: a predicate, split points to determine operands
/// Output: all operands in the predicate
vector<string> get_all_operands(string predicate, vector<int>split_points){
    vector<string> operands;
    for(int k=0;k<split_points.size();k++){
        string op = predicate.substr(split_points[k-1]+1, max(split_points[k]-split_points[k-1] -1, 0));
        if(op!=""){
            operands.push_back(op);
        }
    }
    // Enter last operand
    if(predicate.substr(split_points[split_points.size()-1]+1) != ""){
       operands.push_back(predicate.substr(split_points[split_points.size()-1]+1));
    }
    return operands;
}


/// Determine if operand is a string, e.g. 'Mohan'
/// Input: an operand
/// Output: TRUE if a string enclosed by ', FALSE otherwise
bool is_string(string operand){
    return(operand.substr(0, 1) == "'" && operand.substr(operand.length()-1) == "'");
}


/// Determine if operand is a col_name, e.g. student.id
/// Input: an operand, vector of all possible column names in table
/// Output: TRUE if operand is a column name, FALSE otherwise
bool is_col_name(string operand, vector<string> all_cols){
    return(find(all_cols.begin(), all_cols.end(), operand) != all_cols.end());
}


/// Determine if the operand is an integer, e.g. 30
/// Input: an operand
/// Output: TRUE if an integer, FALSE otherwise
bool is_int(string operand){
    if(operand=="" || operand==" "){
        return false;
    }
    bool is_num = true;
    int deci_pts = 0;
    for(char i : operand){
        if(i == '.'){
            deci_pts++;
        }else if(!isdigit(i)){
            is_num = false;
        }
    }
    return(is_num && (deci_pts == 0));
}


/// Determine if the operand is a float, e.g. 3.0
/// Input: an operand
/// Output: TRUE if a float, FALSE otherwise
bool is_float(string operand){
    if(operand=="" || operand==" "){
        return false;
    }
    bool is_num = true;
    int deci_pts = 0;
    for(char i : operand){
        if(i == '.'){
            deci_pts++;
        }else if(!isdigit(i)){
            is_num = false;
        }
    }
    return(is_num && (deci_pts == 1));
}


/// Determine if predicate is valid
/// Input: predicate, table name, all column names and their data types from table
/// Output: TRUE if predicate seems valid, FALSE otherwise
bool valid_predicate(string predicate, string table_name, string table_cols, string table_info){
    if(predicate == "" || predicate=="*"){
        return true;
    }

    vector<string> all_cols = get_all_cols(table_cols, table_name);
    vector<char> valid_splits = {'(', ')', '>', '<', '=', '!', '|', '&', '+', '-', '*', '/'};
    vector<int> split_points = get_split_points(predicate, valid_splits);
    vector<string> operands = get_all_operands(predicate, split_points);

    // Invalid operand in predicate
    for(string op : operands){
        if(!(is_string(op) || is_col_name(op, all_cols) || is_int(op) || is_float(op))){
            throw_error(19, "invalid predicate given");
        }
    }
    return true;
}


/// Get the data type of operand: int,float,varchar or bool
/// Input: an operand, all variables in table and their data types
/// Output: data type of operand
string get_type(string operand, vector<string> vars, vector<string> var_types){
    if(is_int(operand)){
        return(string("int"));
    }
    if(is_float(operand)){
        return(string("float"));
    }
    if(is_string(operand)){
        return(string("varchar"));
    }
    int index_of_var = find(vars.begin(), vars.end(), operand) - vars.begin();
    if(index_of_var < vars.size()){
        string type_var = var_types[index_of_var % var_types.size()];
        return(type_var);
    }
    // If none of the types, then bool
    return("bool");
}


/// Check if valid operations are applied in predicate
/// Input: a pair of operands, an operator
/// Output: TRUE if opperation is possible, FALSE otherwise
bool valid_operation(string op1_type, string op2_type, string operator_to_use){
    if(op1_type != op2_type){
        return false;
    }
    if(op1_type == "varchar"){
        if(operator_to_use != "=" && operator_to_use != "!"){
            return false;
        }
    }else if(op1_type == "float" || op1_type == "int"){
        if(operator_to_use == "|" || operator_to_use == "&"){
            return false;
        }
    }else if(op1_type == "bool"){
        if(operator_to_use != "|" && operator_to_use != "&"){
            return false;
        }
    }
    return true;
}


/// Perform operations for floats
/// Input: a pair of operands, an operator
/// Output: evaluted answer
string get_result_float(float op_conv1, float op_conv2, string operator1){
    float ans = 0;
    bool ans2 = true, ans2_reqd = false;
    switch(operator1[0]){
        case '+':
            ans = op_conv1 + op_conv2;
            break;
        case '-':
            ans = max(op_conv1 - op_conv2, float(0.0));
            break;
        case '*':
            ans = op_conv1 * op_conv2;
            break;
        case '/':
            if((op_conv2 - 0.0) < 0.000001){
                throw_error(20, "possible division by zero");
            }
            ans = op_conv1 / op_conv2;
            break;
        case '>':
            ans2 = op_conv1 > op_conv2;
            ans2_reqd = true;
            break;
        case '<':
            ans2 = op_conv1 < op_conv2;
            ans2_reqd = true;
            break;
        case '=':
            ans2 = ((op_conv1 - op_conv2) < 0.000001);
            ans2_reqd = true;
            break;
        case '!':
            ans2 = ((op_conv1 - op_conv2) > 0.000001);
            ans2_reqd = true;
            break;
    }
    if(ans2_reqd){
        if(ans2){
            return("true");
        }else{
            return("false");
        }
    }
    return(to_string(ans));
}


/// Perform operations for ints
/// Input: a pair of operands, an operator
/// Output: evaluted answer
string get_result_int(int op_conv1, int op_conv2, string operator1){
    int ans = 0;
    bool ans2 = true, ans2_reqd = false;
    switch(operator1[0]){
        case '+':
            ans = op_conv1 + op_conv2;
            break;
        case '-':
            ans = max(op_conv1 - op_conv2, 0);
            break;
        case '*':
            ans = op_conv1 * op_conv2;
            break;
        case '/':
            if((op_conv2 - 0.0) < 0.000001){
                throw_error(21, "possible division by zero");
            }
            ans = op_conv1 / op_conv2;
            break;
        case '>':
            ans2 = op_conv1 > op_conv2;
            ans2_reqd = true;
            break;
        case '<':
            ans2 = op_conv1 < op_conv2;
            ans2_reqd = true;
            break;
        case '=':
            ans2 = op_conv1 == op_conv2;
            ans2_reqd = true;
            break;
        case '!':
            ans2 = op_conv1 != op_conv2;
            ans2_reqd = true;
            break;
    }
    if(ans2_reqd){
        if(ans2){
            return("true");
        }else{
            return("false");
        }
    }
    return(to_string(ans));
}


/// Perform operations for bools
/// Input: a pair of operands, an operator
/// Output: evaluted answer
string get_result_bool(bool op_conv1, bool op_conv2, string operator1){
    bool ans = true;
    switch(operator1[0]){
        case '&':
            ans = op_conv1 && op_conv2;
            break;
        case '|':
            ans = op_conv1 | op_conv2;
            break;
    }
    if(ans){
        return("true");
    }else{
        return("false");
    }
}


/// Perform operations for strings
/// Input: a pair of operands, an operator
/// Output: evaluted answer
string get_result_string(string op_conv1,string op_conv2,string operator1){
    if(op_conv1.substr(0, 1) == "'"){
        op_conv1 = op_conv1.substr(1, op_conv1.length()-2);
    }
    if(op_conv2.substr(0, 1) == "'"){
        op_conv2 = op_conv2.substr(1, op_conv2.length()-2);
    }
    string ans="";
    bool ans2 = true, ans2_reqd = false;
    switch(operator1[0]){
        case '=':
            ans2 = (op_conv1 == op_conv2);
            ans2_reqd = true;
            break;
        case '!':
            ans2 = (op_conv1 != op_conv2);
            ans2_reqd = true;
            break;
    }
    if(ans2_reqd){
        if(ans2){
            return("true");
        }else{
            return("false");
        }
    }
    return ans;
}


/// Perform operations for according to data type
/// Input: a pair of operands, an operator, their data types, the data tuple to satisfy predicate, all columns in table
/// Output: evaluted answer
string perform_operation(string op1, string op2, string operator1, string data_type, string data_tuple,
    vector<string> all_cols){
    vector<string> data_tuple_vec = get_cols(data_tuple);
    for(int i=0;i<all_cols.size();i++){
        // Replace variable name by value from data tuple
        if(all_cols[i] == op1){
            op1 = data_tuple_vec[i % (all_cols.size()/2)];
        }if(all_cols[i] == op2){
            op2 = data_tuple_vec[i % (all_cols.size()/2)];
        }
    }
    if(data_type == "float"){
        float op_conv1 = stof(op1);
        float op_conv2 = stof(op2);
        return (get_result_float(op_conv1,op_conv2,operator1));
    }else if(data_type == "int"){
        int op_conv1 = stoi(op1);
        int op_conv2 = stoi(op2);
        return (get_result_int(op_conv1,op_conv2,operator1));
    }else if(data_type == "bool"){
        bool op_conv1 = (op1 == "true" ?  true: false);
        bool op_conv2 = (op2 == "true" ?  true: false);
        return (get_result_bool(op_conv1,op_conv2,operator1));
    }else{
        string op_conv1 = op1;
        string op_conv2 = op2;
        return (get_result_string(op_conv1,op_conv2,operator1));
    }
}


/// If data tuple satisfies prdicate, return true
/// Input: predicate, data tuple to test, table name, column names and types
/// Output: TRUE if predicate applies for data tuple, FALSE otherwise
bool satisfies_predicate(string predicate, string data_tuple, string table_name, string table_cols, string table_info){
    if(predicate == "" || predicate=="*"){
        return true;
    }
    vector<string> all_cols = get_all_cols(table_cols, table_name);
    vector<string> col_types = get_cols(table_info);

    vector<char> valid_splits = {'(', ')', '>', '<', '=', '!', '|', '&', '+', '-', '*', '/'};
    vector<int> split_points = get_split_points(predicate, valid_splits);
    vector<string> operands = get_all_operands(predicate, split_points);

    vector<string> vars_in_pred, ints_in_pred, floats_in_pred, strings_in_pred;
    for(string op : operands){
        if(is_string(op)){
            strings_in_pred.push_back(op);
            continue;
        }
        if(is_col_name(op, all_cols)){
            vars_in_pred.push_back(op);
            continue;
        }
        if(is_int(op)){
            ints_in_pred.push_back(op);
        }
        if(is_float(op)){
            floats_in_pred.push_back(op);
        }
    }
    string post_predicate = convert_to_postfix(predicate);
    int op_num=0;
    // Stack to keep storing operands as we perform opoerations
    stack<string>operands_in_order;

    for(int k=0;k<post_predicate.size();k++){
        if(IsOperator(post_predicate[k])){
            // Not enough operands
            if(operands_in_order.size() < 2){
                throw_error(22, "invalid predicate given");
            }
            string op2 = operands_in_order.top();
            operands_in_order.pop();
            string op1 = operands_in_order.top();
            operands_in_order.pop();
            // Get data types
            string op1_type = get_type(op1, all_cols, col_types);
            string op2_type = get_type(op2, all_cols, col_types);
            if(!valid_operation(op1_type, op2_type, post_predicate.substr(k, 1))){
                throw_error(23, "invalid operation in predicate");
            }
            string ans = perform_operation(op1, op2, post_predicate.substr(k, 1), op1_type, data_tuple, all_cols);
            // Push result back to stack
            operands_in_order.push(string(ans));
        }else{
            // Push operands to stack
            operands_in_order.push(post_predicate.substr(k, operands[op_num].length()));
            k += operands[op_num].length()-1;
            op_num++;
        }
    }

    if(operands_in_order.size()!=1){
        throw_error(24, "invalid predicate given");
    }
    string result = operands_in_order.top();
    operands_in_order.pop();

    if(result == "true"){
        return true;
    }else if(result == "false"){
        return false;
    }else{
        throw_error(25, "invalid predicate given");
    }
    return false;
}


/// Check if table name is valid
/// Input: a table name
/// Output: TRUE if name is a valid table name
bool valid_table_name(string table_name){
    if(table_name == ""){
        return false;
    }
    for(int q=0; q<table_name.length(); q++){
        if(not_an_alphanumeric(table_name[q]) && table_name[q] != '_'){
            return false;
        }
    }
    return true;
}


/// Check if column name is valid
/// Input: a column name
/// Output: TRUE if name is a valid column name
bool valid_column_name(string col_name){
    if(col_name == ""){
        return false;
    }
    for(auto chr : col_name){
        if(chr == '{' || chr == '}'){
            return false;
        }
        if(not_an_alphanumeric(chr) && chr!='_'){
            return false;
        }
    }
    return true;
}


/// Throw error if data in table is invalid
/// Input: a table
/// Output: None
void verify_table_contents(vector<string> table){
    if(table.size() < 2){
        throw_error(26, "invalid data encountered");
    }
    if(!valid_table_name(table[0])){
        throw_error(27, string("invalid table name: ") + string(table[0]));
    }
    vector<string> col_names = split_string_by_char(table[1], " ");
    int num_cols = col_names.size();
    for(int k=0; k<num_cols;k++){
        if(!valid_column_name(col_names[k])){
            throw_error(28, string("invalid column name: ") + string(col_names[k]));
        }
    }
    vector<string> col_types = split_string_by_char(table[2], " ");
    int num_col_types = col_types.size();
    if(num_cols != num_col_types){
        throw_error(29, "invalid data encountered");
    }
    for(auto col_type: col_types){
        if(col_type != "int"  && col_type != "varchar" && col_type != "float"){
            throw_error(29, "invalid data encountered");
        }
    }
    vector<string> one_line;
    for(int i=3;i<table.size();i++){
        one_line = split_string_by_char(table[i], " ");
        if(one_line.size() != num_cols){
            throw_error(30, "invalid data encountered");
        }
        for(int j=0; j<num_col_types; j++){
            if(col_types[j] == "int"){
                if(!is_int(one_line[j])){
                    throw_error(31, "invalid data encountered");
                }
            }else if(col_types[j] == "float"){
                if(!is_float(one_line[j])){
                    throw_error(32, "invalid data encountered");
                }
            }
        }
    }
}


/// Print out newline for console
/// Input: None
/// Output: 0 if successful
int newline(){
    cout << endl << ">>> ";
    return 0;
}


/// Print the exit message when input stops
/// Input: None
/// Output: None
void exit_program(){
    newline();
    cout << endl << "Good Bye!" << endl;
    cout << "Exiting terminal...";
}


/////////////////////////////////////////////////////////////////
///// RA FUNCIONS ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


/// SELECT function
/// Input: query for function
/// Output: a table
vector<string> select_ra(string query){
    vector<string> ans, table;

    // Prdicate is in between {predicate}
    size_t found = query.find("}");
    string predicate;
    if (found!=std::string::npos){
        predicate = query.substr(0, found);
    }

    // If no() bracket after {} in SEL{predicate}()
    if(query.substr(found+1, 1) != "("){
        throw_error(4, "invalid query");
    }

    // Pass table name to get table
    table = query_parser(query.substr(found+2));

    // Return whole table
    if(predicate == "" || predicate == "*"){
        ans=table;
    }else{
        // The table schema is same
        for(int i=0; i<3;i++){
            ans.push_back(table[i]);
        }

        string post_predicate = convert_to_postfix(predicate);
        bool is_valid_predicate = valid_predicate(predicate,
                                               table[0], table[1], table[2]);
        if(!is_valid_predicate){
            throw_error(5, "invalid predicate for SELECT");
        }

        // Return all data tuples satisfying predicate
        for(int l=3;l<table.size();l++){
            if(satisfies_predicate(predicate, table[l], table[0], table[1], table[2])){
                ans.push_back(table[l]);
            }
        }
    }
    return(ans);
}


/// PROJECT function
/// Input: query for function
/// Output: a table
vector<string> project_ra(string query){
    vector <string> ans, table;

    // Prdicate is in between {predicate}
    size_t found = query.find("}");
    string predicate;
    if (found!=std::string::npos){
        predicate = query.substr(0, found);
    }

    // If no() bracket after {} in PRO{predicate}()
    if(query.substr(found+1, 1) != "("){
        throw_error(6, "invalid query");
    }
    table = query_parser(query.substr(found+2));

    string table_name = table[0];
    vector<string> columns_in_table = get_all_cols(table[1], table_name);
    vector<string> col_types_in_table = get_cols(table[2]);
    vector<string> reqd_columns;
    bool correct_cols = true;

    if(predicate != "*"){
        reqd_columns = split_string_by_char(predicate, ":");
        // Check if projected columns are present in table
        for(auto col : reqd_columns){
            if(find(columns_in_table.begin(), columns_in_table.end(), col) == columns_in_table.end()){
                correct_cols = false;
                throw_error(7, string("column '") + col + string("' not found in table"));
            }
        }
    }else{
        // If predicate = *, we project all columns
        reqd_columns = get_cols(table[1]);
    }
    if(correct_cols){
        int num_cols = col_types_in_table.size();
        // In what order to show columns
        vector<int> cols_to_use;
        for(auto col : reqd_columns){
            for(int j=0;j<columns_in_table.size();j++){
                if(col == columns_in_table[j]){
                    if(j>=num_cols){
                        cols_to_use.push_back(j-num_cols);
                    }else{
                        cols_to_use.push_back(j);
                    }
                }
            }
        }

        if(repeated_columns(reqd_columns)){
            throw_error(8, "repeated column names in PROJECT");
        }

        // Required answer
        ans.push_back(table_name);
        string cols_in_ans="", col_types_in_ans="";
        for(int i=0;i<cols_to_use.size();i++){
            cols_in_ans = cols_in_ans + string(" ") + columns_in_table[cols_to_use[i]];
            col_types_in_ans = col_types_in_ans + string(" ") + col_types_in_table[cols_to_use[i]];
        }
        // Ignore initial " "
        ans.push_back(cols_in_ans.substr(1));
        ans.push_back(col_types_in_ans.substr(1));

        // Get all data in order of column names
        vector<string> split_input;
        string ans_line="";
        for(int k=3; k<table.size();k++){
            split_input = get_cols(table[k]);
            ans_line = "";
            for(auto i :cols_to_use){
                ans_line += string(" ") + split_input[i];
            }
            if(find(ans.begin(), ans.end(), ans_line.substr(1)) == ans.end()){
                ans.push_back(ans_line.substr(1));
            }
        }
    }
    return(ans);
}


/// RENAME function
/// Input: query for function
/// Output: a table
vector<string> rename_ra(string query){
    vector <string> ans;

    // Prdicate is in between {predicate}
    size_t found = query.find("}");
    string predicate;
    if (found!=std::string::npos){
        predicate = query.substr(0, found);
    }

    // If no() bracket after {} in REN{predicate}()
    if(query.substr(found+1, 1) != "("){
        throw_error(9, "invalid query");
    }
    ans = query_parser(query.substr(found+2));

    // table_info has table name followed by column names, if any
    vector<string> table_info = get_new_table_info(predicate);

    bool table_already_present = false;
    for(auto i : all_tables_in_database){
        if(i == table_info[0] &&  i!=ans[0]){
            table_already_present = true;
            break;
        }
    }
    for(auto i : currently_renamed_tables){
        if(i == table_info[0] && i!=ans[0]){
            table_already_present = true;
            break;
        }
    }
    if(table_already_present){
        throw_error(10, string("Table name: '") + table_info[0] + string("' already present"));
    }
    if(!valid_table_name(table_info[0])){
        throw_error(11, string("Table name: '") + table_info[0] + string("' not allowed"));
    }
    if(table_info[0] == ""){
        throw_error(12, "please specify table name for RENAME");
    }
    // Add to currently renamed tables
    currently_renamed_tables.push_back(table_info[0]);
    // New table name
    ans[0] = table_info[0];

    // If column names are present, add them
    if(table_info.size() > 1){
        vector<string> cols_in_original_table = get_cols(ans[1]);
        if(cols_in_original_table.size()!=table_info.size()-1){
            throw_error(13, string("specify all columns or none in RENAME for table ") +
                        table_info[0]);
        }
        vector <string> rename_cols;
        for(int i=1;i<table_info.size();i++){
            if(!valid_column_name(table_info[i])){
                throw_error(39, string("column name '") + table_info[i] + string("' not allowed"));
            }
            rename_cols.push_back(table_info[i]);
        }
        if(repeated_columns(rename_cols)){
            throw_error(14, "repeated column names in RENAME");
        }
        string attrs = "";
        for(int i=1;i<table_info.size();i++){
            attrs += string(" ") + table_info[i];
        }
        ans[1] = attrs.substr(1);
    }
    return(ans);
}


/// UNION function
/// Input: query for function
/// Output: a table
vector<string> union_ra(string query){
    vector <string> ans, tab1, tab2;

    // Split (a,b) to (a, and b)
    int split_here = where_to_split(query);
    string sub_query = query.substr(0, split_here+1);
    tab1 = query_parser(sub_query);
    tab2 = query_parser(query.substr(split_here+1));

    string table_name1 = tab1[0];
    string table_name2 = tab2[0];
    string ans_table_name = table_name1 + string("_union_") + table_name2;

    // Construct table schema
    ans.push_back(ans_table_name);
    ans.push_back(tab1[1]);
    ans.push_back(tab1[2]);
    for(int i=3; i<tab1.size(); i++){
        ans.push_back(tab1[i]);
    }
    vector<string> table_cols1 = split_string_by_char(tab1[1], " ");
    vector<string> table_cols2 = split_string_by_char(tab2[1], " ");
    vector<string> col_types1 = split_string_by_char(tab1[2], " ");
    vector<string> col_types2 = split_string_by_char(tab2[2], " ");


    bool all_cols_same = all_same_cols(table_cols1, table_cols2, col_types1, col_types2);
    if(!all_cols_same){
        throw_error(15, string("UNION operation needs same columns"));
    }

    // Table may have columns in different order
    vector<int> col_order = get_order(table_cols1, table_cols2);
    bool present_before = false;
    vector<string> one_line;
    string entry="";
    for(int i=3; i<tab2.size(); i++){
        present_before = false;
        one_line = split_string_by_char(tab2[i], " ");
        entry = "";
        for(int j: col_order){
            entry  += " " + one_line[j];
        }

        entry = entry.substr(1);
        for(string str : ans){
            if(str == entry){
                present_before = true;
            }
        }
        if(!present_before){
            ans.push_back(entry);
        }
    }
    return(ans);
}


/// SET DIFFERENCE function
/// Input: query for function
/// Output: a table
vector<string> set_diff_ra(string query){
    vector <string> ans, tab1, tab2;

    // Split (a,b) to (a, and b)
    int split_here = where_to_split(query);
    string sub_query = query.substr(0, split_here+1);
    tab1 = query_parser(sub_query);
    tab2 = query_parser(query.substr(split_here+1));

    string table_name1 = tab1[0];
    string table_name2 = tab2[0];
    string ans_table_name = table_name1 + string("_diff_") + table_name2;

    ans.push_back(ans_table_name);
    ans.push_back(tab1[1]);
    ans.push_back(tab1[2]);

    vector<string> table_cols1 = split_string_by_char(tab1[1], " ");
    vector<string> table_cols2 = split_string_by_char(tab2[1], " ");
    vector<string> col_types1 = split_string_by_char(tab1[2], " ");
    vector<string> col_types2 = split_string_by_char(tab2[2], " ");


    bool all_cols_same = all_same_cols(table_cols1, table_cols2, col_types1, col_types2);
    if(!all_cols_same){
        throw_error(16, string("DIF operation needs same columns"));
    }

    // Tables may have columns in different order
    vector<int> col_order = get_order(table_cols1, table_cols2);
    bool present_in_second = false;
    vector<string> one_line;
    string entry="";
    vector<string> tab3;
    for(int i=3; i<tab2.size(); i++){
        one_line = split_string_by_char(tab2[i], " ");
        entry = "";
        for(int j: col_order){
            entry  += " " + one_line[j];
        }
        tab3.push_back(entry.substr(1));
    }
    for(int l=3; l<tab1.size(); l++){
        present_in_second = false;
        for(int m=0;m<tab3.size();m++){
            if(tab1[l] == tab3[m]){
                present_in_second = true;
            }
        }
        if(!present_in_second){
            ans.push_back(tab1[l]);
        }
    }
    return(ans);
}


/// CARTESIAN PRODUCT function
/// Input: query for function
/// Output: a table
vector<string> cart_prod_ra(string query){
    vector <string> ans, tab1, tab2;

    // Split (a,b) to (a, and b)
    int split_here = where_to_split(query);
    string sub_query = query.substr(0, split_here+1);
    tab1 = query_parser(sub_query);
    tab2 = query_parser(query.substr(split_here+1));

    string table_name1 = tab1[0];
    string table_name2 = tab2[0];
    string ans_table_name = table_name1 + string("_prod_") + table_name2;
    vector<string> table_cols1 = split_string_by_char(tab1[1], " ");
    vector<string> table_cols2 = split_string_by_char(tab2[1], " ");

    bool any_cols_same = any_col_names_same(table_cols1, table_cols2);
    if(any_cols_same){
        throw_error(17, string("CRP operation needs different column names"));
    }
    // Construct table schema
    ans.push_back(ans_table_name);
    ans.push_back(tab1[1] + " " + tab2[1]);
    ans.push_back(tab1[2] + " " + tab2[2]);

    for(int i=3; i<tab1.size(); i++){
        for(int j=3;j<tab2.size();j++){
            ans.push_back(tab1[i] + " " + tab2[j]);
        }
    }
    return(ans);
}
