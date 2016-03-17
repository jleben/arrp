#include "modulo_avoidance.hpp"

#include <iostream>
#include <unordered_set>

using namespace std;

namespace stream {
namespace polyhedral {

void offset_array_relation(array_relation & relation, int offset)
{
    relation.matrix.constant(0) += offset;
}

struct access_info
{
    isl::set domain { nullptr };
    isl::map relation { nullptr };
    int min_a0 = 0;
    int max_a0 = 0;
    int max_offset = 0;
    bool needs_modulo = false;
};

access_info is_candidate_array(array_ptr array, stmt_ptr stmt, const schedule & sched,
                               const model_summary & model,
                               isl::printer & printer)
{
    access_info access;

    if (!array->is_infinite)
        return access;

    int buf_size = array->buffer_size[0];

    if (buf_size < 2)
        return access;

    auto stmt_sched = sched.period.in_domain(stmt->domain);
    auto stmt_sched_domain = stmt_sched.domain().set_for(stmt->domain.get_space());

    auto access_space = isl::space::from(stmt->domain.get_space(), array->domain.get_space());

    auto write = model.write_relations.map_for(access_space);
    auto read = model.read_relations.map_for(access_space);
    access.relation = write | read;
    access.domain = access.relation(stmt_sched_domain);

    cout << "..Accessed array:";
    printer.print(access.domain);
    cout << endl;

    {
        auto a0 = access.domain.get_space()(isl::space::variable, 0);
        access.min_a0 = access.domain.minimum(a0).integer() - array->period_offset;
        access.max_a0 = access.domain.maximum(a0).integer() - array->period_offset;
    }


    int period_increment = array->period % buf_size;
    if (period_increment == 0)
        access.max_offset = 0;
    else if (buf_size % period_increment == 0)
        access.max_offset = ((buf_size / period_increment) - 1) * period_increment;
    else
        // FIXME: Not always true
        access.max_offset = buf_size - 1;

    cout << "..Accessed range: [" << access.min_a0 << "," << access.max_a0 << "]"
         << " + [0," << access.max_offset << "]" << endl;

    if ( access.max_offset + access.max_a0 >= buf_size )
    {
        access.needs_modulo = true;
    }

    return access;
}

void avoid_modulo(schedule & sched, model & m, bool split_statements)
{
    cout << "### MODULO AVOIDANCE ### " << endl;

    model_summary ms(m);
    isl::printer printer(m.context);

    auto sched_stmt_domains = sched.period.domain();
    for (auto & array : m.arrays)
    {
        if (!array->is_infinite)
            continue;

        auto written = ms.write_relations
                .in_range(array->domain)(sched_stmt_domains);
        auto read = ms.read_relations
                .in_range(array->domain)(sched_stmt_domains);

        auto accessed = (written | read).set_for(array->domain.get_space());

        auto a0 = accessed.get_space().var(0);
        int min_a0 = accessed.minimum(a0).integer();
        int buf_size = array->buffer_size[0];
        array->period_offset = (min_a0 / buf_size) * buf_size;
    }

    isl::union_map new_sched(m.context);

    vector<stmt_ptr> new_stmts;

    for (auto & stmt : m.statements)
    {
        unordered_set<array_ptr> checked_arrays;
        unordered_map<array_ptr, access_info> candidate_arrays;

        auto stmt_sched = sched.period.in_domain(stmt->domain);
        if (!stmt->is_infinite)
        {
            new_sched = new_sched | stmt_sched;
            continue;
        }

        cout << "Statement " << stmt->name << endl;

#if 0
        auto stmt_sched_domain = stmt_sched.domain();
        auto write_rel = ms.write_relations.in_domain(stmt->domain).in_range(array->domain);
        auto array_sched_domain =
                write_rel(stmt_sched_domain)
                .set_for(array->domain.get_space());
        cout << "..Accessed array:" << endl;
        printer.print(array_sched_domain);
        cout << endl;

        int min_a0, max_a0;
        {
            auto a0 = array_sched_domain.get_space()(isl::space::variable, 0);
            min_a0 = array_sched_domain.minimum(a0).integer() - array->period_offset;
            max_a0 = array_sched_domain.maximum(a0).integer() - array->period_offset;
            cout << "..Accessed range: [" << min_a0 << "," << max_a0 << "]" << endl;
        }

        auto is_candidate_array = [&](array_ptr array) -> bool
        {
            if (!array->is_infinite)
                return false;

            int buf_size = array->buffer_size[0];

            if (buf_size < 2)
                return false;

            if (max_a0 == min_a0)
                return false;

            // FIXME: Not always true
            if (array->period % buf_size != 0)
                return true;

            int max_array_offset =
                    ((buf_size - 1) / array->period) * array->period;

            if ( max_array_offset + max_a0 < buf_size )
                return false;

            return true;
        };
#endif

        // Check write relation
        {
            auto array = stmt->write_relation.array;
            if (array)
            {
                auto info = is_candidate_array(array, stmt, sched, ms, printer);
                if (info.needs_modulo)
                    candidate_arrays.emplace(array, info);
                checked_arrays.insert(array);
            }
        }

        // Check read relations
        for (auto relation : stmt->read_relations)
        {
            auto array = relation->array;
            if (checked_arrays.find(array) != checked_arrays.end())
                continue;
            auto info = is_candidate_array(array, stmt, sched, ms, printer);
            if (info.needs_modulo)
                candidate_arrays.emplace(array, info);
            checked_arrays.insert(array);
        }

        bool keep_going = true;

        if (candidate_arrays.size() == 0)
        {
            cout << "..Does not need modulo." << endl;
            keep_going = false;
        }
        else
            stmt->streaming_needs_modulo = true;

        if (keep_going && candidate_arrays.size() > 1)
        {
            cout << "..Needs modulo for more than 1 array access." << endl;
            keep_going = false;
        }

        if (keep_going)
        {
            auto sched_domain = stmt_sched.domain().set_for(stmt->domain.get_space());
            if (sched_domain.is_singleton())
            {
                cout << "..Needs modulo, but domain is singleton." << endl;
                keep_going = false;
            }
        }

        // FIXME: Skip if a single iteration has multiple accesses
        // that would have to be split.

        if (!keep_going || !split_statements)
        {
            new_sched = new_sched | stmt_sched;
            continue;
        }

        stmt->streaming_needs_modulo = false;

        auto array = candidate_arrays.begin()->first;
        auto access = candidate_arrays.begin()->second;

        string offset_name = array->name + "_offset";
        m.phase_ids.emplace(offset_name, array);

        int buf_size = array->buffer_size[0];
        assert(buf_size >= 2);

        int min_a0 = access.min_a0;
        int max_a0 = access.max_a0 + access.max_offset;
        int num_parts = (int) std::ceil((max_a0 - min_a0 + 1) / (float) buf_size);

        cout << "..# Parts = " << num_parts << endl;
        assert(num_parts >= 2);

        auto array_sched_domain = access.domain;

        auto offset_space = array_sched_domain.get_space();
        offset_space.insert_dimensions(isl::space::parameter, 0, 1);
        offset_space.set_name(isl::space::parameter, 0, offset_name);
        auto offset_set = isl::set::universe(offset_space);
        auto offset = offset_space(isl::space::parameter, 0);
        offset_set.add_constraint(offset >= 0);
        offset_set.add_constraint(offset < buf_size);

        sched.params = sched.params | offset_set.parameters();

        array_sched_domain = array_sched_domain & offset_set;

        int lower_bound = (min_a0 / buf_size) * buf_size;

        for (int i = 0; i < num_parts; ++i, lower_bound += buf_size)
        {
            auto array_part = array_sched_domain;
            auto a0 = array_part.get_space()(isl::space::variable, 0);
            array_part.add_constraint(a0 + offset >= lower_bound);
            array_part.add_constraint(a0 + offset < lower_bound + buf_size);

            cout << "..Array Part " << i << ":" << endl;
            printer.print(array_part);
            cout << endl;

            auto stmt_part = access.relation.inverse()(array_part);

            cout << "..Stmt Part " << i << ":" << endl;
            printer.print(stmt_part);
            cout << endl;

            auto new_stmt = make_shared<statement>(*stmt);
            new_stmt->domain = stmt_part;
            new_stmt->name = stmt->name + "_p" + std::to_string(i);
            new_stmt->array_access_offset.emplace(array.get(), -lower_bound);

            new_stmts.push_back(new_stmt);

            auto sched_part = stmt_sched.in_domain(stmt_part);

            sched_part.for_each([&](isl::map & map){
                map.set_name(isl::space::input, new_stmt->name);
                new_sched = new_sched | map;
                return true;
            });
        }
    }

#if 1
    sched.period = new_sched;

    cout << "New period schedule:" << endl;
    new_sched.for_each([&](const isl::map & m){
        printer.print(m);
        cout << endl;
        return true;
    });

    if (sched.params.is_empty())
        sched.params = isl::set::universe(sched.params.get_space());

    cout << "Parameters:" << endl;
    printer.print(sched.params);
    cout << endl;

    m.statements.insert(m.statements.end(),
                        new_stmts.begin(), new_stmts.end());
#endif
}

}
}
