#include"database.h"

 //buy_queue access last element (ascending order)!
// sell_queue access last element (descinding order)!

void place_order(int stock_id,int buy_flag,int sell_flag,int units,int price,std::string status,int user_id){
    insert_order(order(stock_id,buy_flag,sell_flag,units,price,status,user_id));
    std::cout<<"Order Placed!"<<std::endl;
}

void test_place_orders(){

    srand(time(nullptr));
    for(int i=0;i<30;i++){
        int random_price = std::rand()%50;
        int buy_flag = std::rand()%2;
        int sell_flag;
        int user_id = std::rand()%10;
        int stock_id = std::rand()%3;
        int units = std::rand()%100;
        std::string status = "Placed";
        buy_flag? sell_flag=0:sell_flag=1;
        place_order(stock_id,buy_flag,sell_flag,units,random_price,status,user_id);
    }
}

void execute_orders(){
    for(auto &stock: stock_queue){
        
        while(!stock.buy_queue.empty() && !stock.sell_queue.empty() && stock.buy_queue.back().price>stock.sell_queue.back().price){
            order order_for_buying = stock.buy_queue.back();
            order order_for_selling = stock.sell_queue.back();

            if(order_for_buying.units == order_for_selling.units){
                std::cout<<"Order Executed Completely!"<<std::endl;
                order_for_buying.status="Fully Executed";
                order_for_selling.status = "Fully Executed";
                update_status(order_for_buying);
                update_status(order_for_selling);
                transaction_insert(order_for_buying,order_for_selling);
                stock.buy_queue.pop_back();
                stock.sell_queue.pop_back();
            }
            else if(order_for_buying.units > order_for_selling.units){
                std::cout<<"Sell Order Executed Completely but Buy Order Executed Partially!"<<std::endl;
                order_for_buying.units = order_for_buying.units - order_for_selling.units;
                order_for_buying.status="Partially Executed";
                order_for_selling.status ="Fully Executed";
                update_status(order_for_buying);
                update_status(order_for_selling);
                transaction_insert(order_for_buying,order_for_selling);
                stock.sell_queue.pop_back();
                stock.buy_queue[stock.buy_queue.size()-1] = order_for_buying;

            }
            else if(order_for_buying.units < order_for_selling.units){
                std::cout<<"Buy Order Executed Completely but Sell Order Executed Partially!"<<std::endl;
                order_for_selling.units = order_for_selling.units - order_for_buying.units;
                order_for_buying.status ="Fully Executed";
                order_for_selling.status="Partially Executed";
                update_status(order_for_buying);
                update_status(order_for_selling);
                transaction_insert(order_for_buying,order_for_selling);
                stock.buy_queue.pop_back();
                stock.sell_queue[stock.sell_queue.size()-1]=order_for_selling;
            }
        }
    }
}

int main(){

    
    test_place_orders();
    std::cout<<"Orders Placed!"<<std::endl;
    std::cout<<"Orders Executing!"<<std::endl;
    execute_orders();
    std::cout<<"Orders are finished executing!"<<std::endl;

}