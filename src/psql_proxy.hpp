#ifndef PSQL_PROXY_HPP
#define PSQL_PROXY_HPP

#include <list>
#include <pair>

#include <boost/optional>

struct RaceID {
    using key = std::string;
    using value = std::string;
    using elem = std::pair<key, value>;
    std::list<elem> value;
}

struct QueryHRTansyou {
    using payout = std::pair<std::string, long>;
    using result_type = boost::optional< std::list<payout> >;

    result_type operator()(pqxx::connection& c, const RaceID& race_id) {
        pqxx::work work{c};

        std::string query = generate_query(race_id);

        std::list<payout> res;
        pqxx::result r = work.exec(query);

        // Not really needed, since we made no changes, but good habit to be
        // explicit about when the transaction is done.
        work.commit();

        if (r.size() == 1) {
            const auto& row = r.front();
            if (!row["paytansyoumaban1"].is_null()) {
                res.push_back(payout( row["paytansyoumaban1"].c_str(), row["paytansyopay1"].as<long>() ));
            }
            else return boost:none;

            if (!row["paytansyoumaban2"].is_null()) {
                res.push_back(payout( row["paytansyoumaban2"].c_str(), row["paytansyopay2"].as<long>() ));
            }

            if (!row["paytansyoumaban3"].is_null()) {
                res.push_back(payout( row["paytansyoumaban3"].c_str(), row["paytansyopay3"].as<long>() ));
            }

        }

        else {
            else return boost:none;
        }

        return res;

    ERROR:
    }

private:

    std::string generate_query(const RaceID& id) {
        assert(race_id.value.size() == 6);

        std::stringstream ss;

        ss << "SELECT paytansyoumaban1, paytansyopay1, paytansyoumaban2, paytansyopay2, paytansyoumaban3, paytansyopay3"
            " FROM public.n_harai"

        const auto& const_it = race_id.value.begin();
        ss << " WHERE " << race_id.value.front().first << "='" << race_id.value.front().second << "'";
        for (const auto& elem = race_id.value) {
            ss << " AND " << const_it.first << "='" << const_it.second << "'";
        }

        ss << " ORDER BY year DESC, jyocd DESC, kaiji DESC, nichiji DESC, racenum DESC"

        return ss.str();
    }
};

class PSQLProxy {
    pqxx::connection connection_;

public:
    PSQLProxy() : connection_("host=192.168.11.2 dbname=everydb2 user=postgres port=5433 password=password") { };

    template<class T>
    auto operator()( const RaceID& race_id ) -> T::result_type 
    {
        return T{}.operator(connection_, id);
    };
};

#endif
