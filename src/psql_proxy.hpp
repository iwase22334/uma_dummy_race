#ifndef PSQL_PROXY_HPP
#define PSQL_PROXY_HPP

#include <pqxx/pqxx>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

#include <memory>
#include <string>

struct QueryHRTansyou {
    using ptree = boost::property_tree::ptree;
    using result_type = std::shared_ptr< ptree >;

    result_type operator()(pqxx::connection& c, const ptree& race_id) noexcept(false) {
        if (!is_valid( race_id )) throw std::runtime_error(R"({ "error": "Invalid race_id" })");

        // Ask postgres
        pqxx::work work{c};
        std::string query = generate_query(race_id);
        pqxx::result pq_result = work.exec(query);
        // Not really needed, since we made no changes, but good habit to be
        // explicit about when the transaction is done.
        work.commit();

        auto res = std::make_shared< ptree >();

        if (pq_result.size() == 1) {
            const auto& row = pq_result.front();
            if (!row["paytansyoumaban1"].is_null()) {
                res->put( row["paytansyoumaban1"].c_str(), row["paytansyopay1"].as<long>() );
            }
            else { 
                throw std::runtime_error(R"({ "error": "No target data in record" })"); 
            }

            if (!row["paytansyoumaban2"].is_null()) {
                res->put( row["paytansyoumaban2"].c_str(), row["paytansyopay2"].as<long>() );
            }

            if (!row["paytansyoumaban3"].is_null()) {
                res->put( row["paytansyoumaban3"].c_str(), row["paytansyopay3"].as<long>() );
            }

        }

        else {
             throw std::runtime_error(R"({ "error": "Record not found" })"); 
        }

        return res;

    }

private:
    bool is_valid(const ptree& id) const {
        auto is_number = [](const std::string& s) -> bool {
            return !s.empty() && std::find_if(s.begin(), 
                s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
        };

        const std::list< std::pair<std::string, int> > value_len_list {
            {"year"     , 4},
            {"monthday" , 4},
            {"jyocd"    , 2},
            {"kaiji"    , 2},
            {"nichiji"  , 2},
            {"racenum"  , 2}
        };

        for (const auto& value_len: value_len_list) {
            if (boost::optional<std::string> value = id.get_optional<std::string>(value_len.first)) {
                if (!is_number(value.get())) 
                    return false;
                if (value.get().size() != value_len.second) 
                    return false;
            }

            else {
                return false;
            }

        }

        return true;
    }

    std::string generate_query(const ptree& id) const {
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
    auto operator()( const ptree& race_id ) -> typename T::result_type
    {
        return T{}.operator()(connection_, race_id);
    };
};

#endif
