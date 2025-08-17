#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stack>
#include <algorithm>
using namespace std;

struct User {
    string username, password, role;
};

struct Event {
    int id;
    string name, date, time, type, location;
    int duration;
};

bool validDate(const string &d){ return d.size()==10 && d[2]=='-' && d[5]=='-'; }
bool validTime(const string &t){ return t.size()==5 && t[2]==':'; }

int toMinutes(const string &t){
    int h=stoi(t.substr(0,2)), m=stoi(t.substr(3,2));
    return h*60+m;
}

void loadEvents(vector<Event> &e){
    e.clear();
    ifstream f("events.csv");
    string line; getline(f,line);
    while(getline(f,line)){
        stringstream ss(line);
        Event ev; string temp;
        getline(ss,temp,','); if(temp.empty()) continue;
        ev.id=stoi(temp); getline(ss,ev.name,',');
        getline(ss,ev.date,','); getline(ss,ev.time,',');
        getline(ss,ev.type,','); getline(ss,ev.location,',');
        getline(ss,temp,','); ev.duration=stoi(temp);
        e.push_back(ev);
    }
}

//saves events in .csv file
void saveEvents(const vector<Event>&e){
    ofstream f("events.csv");
    f<<"id,name,date,time,type,location,duration_hours\n";
    for(auto &x:e)
        f<<x.id<<","<<x.name<<","<<x.date<<","<<x.time<<","<<x.type
        <<","<<x.location<<","<<x.duration<<"\n";
}

//checks whether there is conflict of dates and time or not
bool conflict(const vector<Event>&ev, const Event&n){
    for(auto &e:ev){
        if(e.date==n.date){
            int s1=toMinutes(e.time), e1=s1+e.duration*60;
            int s2=toMinutes(n.time), e2=s2+n.duration*60;
            if(max(s1,s2)<min(e1,e2)) return true;
        }
    }
    return false;
}

stack<vector<Event>> hist;
void backup(const vector<Event>&e){ hist.push(e); }
void undo(vector<Event>&e){
    if(hist.empty()){ cout<<"Nothing to undo.\n"; return;}
    e=hist.top(); hist.pop();
    saveEvents(e);
    cout<<"Undone.\n";
}

//adding a event 
void addEvent(vector<Event> &e){
    Event n;
    cout<<"Name: "; getline(cin,n.name);
    cout<<"Date(DD-MM-YYYY): "; getline(cin,n.date);
    cout<<"Time(HH:MM): "; getline(cin,n.time);
    cout<<"Type: "; getline(cin,n.type);
    cout<<"Location: "; getline(cin,n.location);
    cout<<"Duration(hours): "; cin>>n.duration; cin.ignore();

    if(!validDate(n.date)||!validTime(n.time)) {
        cout<<"Invalid format.\n"; return;
    }
    n.id = (e.empty()?1:e.back().id+1);
    if(conflict(e,n)){ cout<<"Conflict! Choose another slot.\n"; return;}

    backup(e);
    e.push_back(n);
    saveEvents(e);
    cout<<"Added.\n";
}

//edit existing event
void editEvent(vector<Event> &e){
    int id; cout<<"Enter ID: "; cin>>id; cin.ignore();
    auto it=find_if(e.begin(),e.end(),[&](Event &x){return x.id==id;});
    if(it==e.end()){ cout<<"Not found.\n"; return;}
    Event &ev=*it;
    string t;
    cout<<"New name("<<ev.name<<"): "; getline(cin,t); if(!t.empty())ev.name=t;
    cout<<"New date("<<ev.date<<"): "; getline(cin,t); if(!t.empty()) ev.date=t;
    cout<<"New time("<<ev.time<<"): "; getline(cin,t); if(!t.empty()) ev.time=t;
    cout<<"New type("<<ev.type<<"): "; getline(cin,t); if(!t.empty()) ev.type=t;
    cout<<"New location("<<ev.location<<"): "; getline(cin,t); if(!t.empty()) ev.location=t;
    cout<<"New duration("<<ev.duration<<"): "; getline(cin,t); if(!t.empty()) ev.duration=stoi(t);

    if(conflict(e,ev)){ cout<<"Conflict detected. Cancelled.\n"; return;}
    backup(e);
    saveEvents(e);
    cout<<"Updated.\n";
}

//delete existing event
void deleteEvent(vector<Event>&e){
    int id; cout<<"Enter ID: "; cin>>id; cin.ignore();
    auto it=remove_if(e.begin(),e.end(),[&](Event&x){return x.id==id;});
    if(it==e.end()){ cout<<"Not found.\n"; return;}
    backup(e);
    e.erase(it,e.end());
    saveEvents(e);
    cout<<"Deleted.\n";
}

void viewAll(const vector<Event>&e){
    for(auto &x:e){
        cout<<x.id<<" | "<<x.name<<" | "<<x.date<<" "<<x.time<<" ("<<x.duration<<"hr) | "
            <<x.type<<" | "<<x.location<<"\n";
    }
}

void searchEv(const vector<Event>&e){
    cout<<"Keyword: "; string k; getline(cin,k);
    string low=k; transform(low.begin(),low.end(),low.begin(),::tolower);
    for(auto &x:e){
        string n=x.name, t=x.type;
        transform(n.begin(),n.end(),n.begin(),::tolower);
        transform(t.begin(),t.end(),t.begin(),::tolower);
        if(n.find(low)!=string::npos||t.find(low)!=string::npos)
            cout<<x.id<<" | "<<x.name<<" | "<<x.date<<" "<<x.time<<"\n";
    }
}

void loadUsers(vector<User>&u){
    u.clear();
    ifstream f("users.csv");
    string l; getline(f,l);
    while(getline(f,l)){
        stringstream ss(l); User x;
        getline(ss,x.username,',');
        getline(ss,x.password,',');
        getline(ss,x.role,',');
        u.push_back(x);
    }
}

bool login(User &u){
    vector<User> all; loadUsers(all);
    cout<<"Username:"; getline(cin,u.username);
    cout<<"Password:"; getline(cin,u.password);
    for(auto &x:all){
        if(x.username==u.username && x.password==u.password){ u.role=x.role; return true;}
    }
    return false;
}

void autoCreate(){
    ifstream f("users.csv");
    if(!f.good()){
        ofstream u("users.csv");
        u<<"username,password,role\nadmin,admin123,admin\njohn,john123,general\n";
    }
    ifstream e("events.csv");
    if(!e.good()){
        ofstream c("events.csv");
        c<<"id,name,date,time,type,location,duration_hours\n"
         <<"1,Tech Talk,25-08-2024,10:00,Seminar,Auditorium,1\n"
         <<"2,Workshop on AI,27-08-2024,14:00,Workshop,Lab 3,2\n";
    }
}

int main(){
    autoCreate();
    vector<Event> ev;
    loadEvents(ev);
    User cur;
    if(!login(cur)){ cout<<"Invalid.\n"; return 0;}
    cout<<"Welcome "<<cur.username<<" ("<<cur.role<<")\n";

    while(true){
        cout<<"\n1.View All\n2.Search\n";
        if(cur.role=="admin"){
            cout<<"3.Add\n4.Edit\n5.Delete\n6.Undo\n";
        }
        cout<<"0.Exit\nChoice: ";
        int ch; cin>>ch; cin.ignore();
        if(ch==0)break;
        switch(ch){
            case 1: viewAll(ev); break;
            case 2: searchEv(ev); break;
            case 3: if(cur.role=="admin") addEvent(ev); break;
            case 4: if(cur.role=="admin") editEvent(ev); break;
            case 5: if(cur.role=="admin") deleteEvent(ev); break;
            case 6: if(cur.role=="admin") undo(ev); break;
            default: cout<<"Invalid\n";
        }
        loadEvents(ev);
    }
    return 0;
}
