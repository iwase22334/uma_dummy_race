#ifndef PSQL_PROXY_HPP
#define PSQL_PROXY_HPP

#include <pqxx/pqxx>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

#include <memory>
#include <unordered_map>
#include <iostream>
#include <string>

struct QueryHRTansyou {
    using ptree = boost::property_tree::ptree;
    using result_type = long;

    result_type operator()(pqxx::connection& c, const ptree& json) noexcept(false) {
        boost::optional<const ptree&> json_id  = json.get_child_optional("race_id");
        if (json_id) {
            if (!is_valid_id( json_id.get() )) 
                throw std::runtime_error(R"({ "error": "Invalid race_id" })");
        }
        else {
            throw std::runtime_error(R"({ "error": "No race_id field" })");
        }

        std::cout << "QueryHRTansyou : " << "connect to postgres" << std::endl;
        std::cout << "-----------------" << std::endl;
        for (const auto& elem: json_id.get()) {
            std::cout << elem.first << ": " << elem.second.data() << std::endl;
        }

        // Ask postgres
        pqxx::work work{c};
        std::string query = generate_sql_query(json_id.get());
        pqxx::result pq_result = work.exec(query);
        // Not really needed, since we made no changes, but good habit to be
        // explicit about when the transaction is done.
        work.commit();

        std::cout << "QueryHRTansyou : " << "disconnect fron postgres" << std::endl;

        std::unordered_map<std::string, long> compairson;
        if (pq_result.size() == 1) {
            const auto& row = pq_result.front();

            if (!row["paytansyoumaban1"].is_null() && !row["paytansyopay1"].is_null()) {
                compairson.insert( {row["paytansyoumaban1"].c_str(), std::atoi(row["paytansyopay1"].c_str())} );
            }

            else {
                throw std::runtime_error(R"({ "error": "No target data in record" })");
            }

            if (!row["paytansyoumaban2"].is_null() && !row["paytansyopay2"].is_null()) {
                compairson.insert( {row["paytansyoumaban2"].c_str(), std::atoi(row["paytansyopay2"].c_str())} );
            }

            if (!row["paytansyoumaban3"].is_null() && !row["paytansyopay3"].is_null()) {
                compairson.insert( {row["paytansyoumaban3"].c_str(), std::atoi(row["paytansyopay3"].c_str())} );
            }

        }

        // calculate total of pay out
        long sum = 0;
        for (auto elem: json.get_child("tansyo_vote")) {
            boost::optional<std::string> umaban = elem.second.get_optional<std::string>("umaban");
            boost::optional<int> hyosu = elem.second.get_optional<int>("hyosu");
            if (!umaban || !hyosu)
                throw std::runtime_error(R"({ "error": "Invalid syntax of tansyo_vote" })");

            auto search = compairson.find(umaban.get());
            if (search != compairson.end()) {
                sum += search->second * hyosu.get();
            }

        }

        return sum;

    }

private:
    bool is_valid_id(const ptree& id) const {
        auto is_number = [](const std::string& s) -> bool {
            return !s.empty() && std::find_if(s.begin(), 
                s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
        };

        const std::list< std::pair<std::string, size_t> > value_len_list {
            {"year"     , 4},
            {"monthday" , 4},
            {"jyocd"    , 2},
            {"kaiji"    , 2},
            {"nichiji"  , 2},
            {"racenum"  , 2}
        };

        for (const auto& value_len: value_len_list) {
            if (boost::optional<std::string> value = id.get_optional<std::string>(value_len.first)) {
                if (!is_number(value.get())) {
                    std::cerr << "is_valid: invalid value type" << value.get() << std::endl;
                    return false;
                }

                if (value.get().size() != static_cast<size_t>(value_len.second)) {
                    std::cerr << "is_valid: invalid value size" << value.get() << ": size = " << value.get().size() << std::endl;
                    return false;
                }
            }

            else {
                std::cerr << "is_valid: no key value in tree" << std::endl;
                return false;
            }

        }

        return true;
    }

    std::string generate_sql_query(const ptree& id) const {
        std::stringstream ss;

        ss << "SELECT paytansyoumaban1, paytansyopay1, paytansyoumaban2, paytansyopay2, paytansyoumaban3, paytansyopay3"
            " FROM public.n_harai";
        ss << " WHERE ";

        auto end = std::prev(id.end());
        for (auto it = id.begin(); it != end; ++it) {
            ss << boost::lexical_cast<std::string>((*it).first) 
                << R"(=')" 
                << (*it).second.get_value<std::string>() << R"(')" << " AND ";
        }

        ss << boost::lexical_cast<std::string>(id.back().first) 
            << R"(=')" 
            << id.back().second.get_value<std::string>() << R"(')";

        ss << " ORDER BY year DESC, jyocd DESC, kaiji DESC, nichiji DESC, racenum DESC";

        return ss.str();
    }
};

class PSQLProxy {
    using ptree = boost::property_tree::ptree;
    pqxx::connection connection_;

public:
    PSQLProxy() : connection_("host=192.168.11.2 dbname=everydb2 user=postgres port=5433 password=password") { };

    template<class T>
    auto operator()( const ptree& json ) -> typename T::result_type
    {
        return T{}.operator()(connection_, json);
    };
};

#endif
