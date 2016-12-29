//
// Created by ubuntu on 25.12.16.
//

#include "StorageClient.h"
#include "Query.h"

int const QUERY_SUCCESS = 0;
int const QUERY_FAILURE = 1;


StorageClient::StorageClient(const char* hosts) : hosts(hosts), connected(false) {
    session = cass_session_new();
    cluster = cass_cluster_new();
    cass_cluster_set_contact_points(cluster, hosts);
}

StorageClient::~StorageClient() {
    if (connected) {
        CassFuture* close_future = cass_session_close(session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
    }

    cass_cluster_free(cluster);
    cass_session_free(session);
}

void StorageClient::connect(const char** message) {
    CassFuture* connect_future;
    connect_future = cass_session_connect(session, cluster);
    if (cass_future_error_code(connect_future) == CASS_OK) {
        connected = true;
    } else {
        connected = false;
        if (message) {
            size_t messageLength;
            cass_future_error_message(connect_future, message, &messageLength);
        }
    }

    cass_future_free(connect_future);
}

int StorageClient::perform_query(CassStatement* statement, const CassResult** result, const char** message) {
    int statusCode = QUERY_FAILURE;
    if (!connected) {
        connect(message);
        if (!connected)  {
            return  statusCode;
        }
    }

    CassFuture* result_future = cass_session_execute(session, statement);
    cass_statement_free(statement);

    if (cass_future_error_code(result_future) == CASS_OK) {
        *result = cass_future_get_result(result_future);
        statusCode = QUERY_SUCCESS;
    } else {
        size_t messageLength;
        cass_future_error_message(result_future, message, &messageLength);
    }

    cass_future_free(result_future);

    return statusCode;
}

int StorageClient::perform_batch_query(CassBatchType type, std::vector<CassStatement*>& statements, const char **message) {
    int statusCode = QUERY_FAILURE;
    if (!connected) {
        connect(message);
        if (!connected) {
            return statusCode;
        }
    }

    CassBatch* batch = cass_batch_new(type);

    for (auto it = statements.begin(); it != statements.end(); ++it) {
        cass_batch_add_statement(batch, *it);
        cass_statement_free(*it);
    }

    CassFuture* batch_future = cass_session_execute_batch(session, batch);
    cass_batch_free(batch);

    if (cass_future_error_code(batch_future) == CASS_OK) {
        statusCode = QUERY_SUCCESS;
    } else {
        size_t messageLength;
        cass_future_error_message(batch_future, message, &messageLength);
    }

    cass_future_free(batch_future);

    return statusCode;
}

int StorageClient::polls_get(std::string const* creationDateTime, std::vector<Poll>& result, const char** message) {
    const CassResult* query_result = NULL;

    int limit = 10;
    CassStatement* statement = select_polls_query(limit, creationDateTime);
    int statusCode = perform_query(statement, &query_result, message);
    if (statusCode != QUERY_SUCCESS) {
        return  statusCode;
    }

    if (cass_result_row_count(query_result) > 0) {
        CassIterator *iter = cass_iterator_from_result(query_result);
        while (cass_iterator_next(iter)) {
            const CassRow *row = cass_iterator_get_row(iter);
            const CassValue *value = cass_row_get_column_by_name(row, "id");

            const char *id;
            size_t id_length;
            cass_value_get_string(value, &id, &id_length);

            const char *creationDT;
            size_t creationDateTime_length;
            value = cass_row_get_column_by_name(row, "creationDateTime");
            cass_value_get_string(value, &creationDT, &creationDateTime_length);

            const char *name;
            size_t name_length;
            value = cass_row_get_column_by_name(row, "name");
            cass_value_get_string(value, &name, &name_length);

            Poll poll(std::string(id, id_length));
            poll.creationDateTime = std::string(creationDT, creationDateTime_length);
            poll.name = std::string(name, name_length);

            result.push_back(poll);
        }

        cass_iterator_free(iter);
    }

    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::poll_get(std::string const& id, std::vector<Poll>& result, const char** message) {
    const CassResult* query_result = NULL;

    CassStatement* statement = select_poll_query(id);
    int statusCode = perform_query(statement, &query_result, message);

    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    const CassRow* row = cass_result_first_row(query_result);
    if (row) {
        const CassValue* value = cass_row_get_column_by_name(row, "creationDateTime");

        const char* creationDateTime;
        size_t creationDateTime_length;
        cass_value_get_string(value, &creationDateTime, &creationDateTime_length);

        const char* name;
        size_t name_length;
        value = cass_row_get_column_by_name(row, "name");
        cass_value_get_string(value, &name, &name_length);

        const char* description;
        size_t description_length;
        value = cass_row_get_column_by_name(row, "description");
        cass_value_get_string(value, &description, &description_length);

        const char* author;
        size_t author_length;
        value = cass_row_get_column_by_name(row, "author");
        cass_value_get_string(value, &author, &author_length);

        Poll poll(id);
        poll.creationDateTime = std::string(creationDateTime, creationDateTime_length);
        poll.name = std::string(name, name_length);
        poll.description = std::string(description, description_length);
        poll.author = std::string(author, author_length);

        value = cass_row_get_column_by_name(row, "options");
        std::vector<PollOption> opts(cass_value_item_count(value));

        CassIterator* iter = cass_iterator_from_collection(value);
        while (cass_iterator_next(iter)) {
            value = cass_iterator_get_value(iter);
            CassIterator* opt_iter = cass_iterator_fields_from_user_type(value);

            PollOption option;
            while (cass_iterator_next(opt_iter)) {
                const char* field_name;
                size_t field_name_length;
                cass_iterator_get_user_type_field_name(opt_iter, &field_name, &field_name_length);
                const std::string str(field_name, field_name_length);

                const CassValue* val = cass_iterator_get_user_type_field_value(opt_iter);
                if (str == "name") {
                    const char* optname;
                    size_t optname_lenght;
                    cass_value_get_string(val, &optname, &optname_lenght);
                    option.name = std::string(optname, optname_lenght);
                } else if (str == "id") {
                    int optid;
                    cass_value_get_int32(val, &optid);
                    option.name = optid;
                }
            }
            cass_iterator_free(opt_iter);
            opts.push_back(option);
        }
        poll.options = opts;

        cass_iterator_free(iter);
        cass_result_free(query_result);

        statement = select_poll_votes_query(id);
        statusCode = perform_query(statement, &query_result, message);

        if (statusCode != QUERY_SUCCESS) {
            return  statusCode;
        }

        row = cass_result_first_row(query_result);
        cass_result_free(query_result);

        if (row != NULL) {
            value = cass_row_get_column_by_name(row, "totalVotes");
            cass_value_get_int64(value, &(poll.totalVotes));
        }

        statement = select_poll_option_votes_query(id);
        statusCode = perform_query(statement, &query_result, message);

        if (statusCode != QUERY_SUCCESS) {
            return statusCode;
        }

        iter = cass_iterator_from_result(query_result);
        cass_result_free(query_result);
        while (cass_iterator_next(iter)) {
            const CassRow* optrow = cass_iterator_get_row(iter);
            const CassValue* val = cass_row_get_column_by_name(optrow, "id");
            int optionid;
            cass_value_get_int32(val, &optionid);
            for (auto it = poll.options.begin(); it != poll.options.end(); ++it) {
                if (it->id == optionid) {
                    val = cass_row_get_column_by_name(row, "votes");
                    long long votes;
                    cass_value_get_int64(val, &votes);
                    it->votes = votes;
                    break;
                }
            }
        }

        cass_iterator_free(iter);
        result.push_back(poll);
    }

    return statusCode;
}

int StorageClient::votes_get(std::string const& pollid, std::vector<Vote>& result, const char** message) {
    CassStatement* statement = select_votes_query(pollid);
    const CassResult* query_result = NULL;
    int statusCode = perform_query(statement, &query_result, message);

    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    CassIterator* iter = cass_iterator_from_result(query_result);
    while (cass_iterator_next(iter)) {
        const CassRow* row = cass_iterator_get_row(iter);

        const CassValue* value = cass_row_get_column_by_name(row, "id");
        const char* id;
        size_t id_length;
        cass_value_get_string(value, &id, &id_length);

        value = cass_row_get_column_by_name(row, "creationDateTime");
        const char* createDateTime;
        size_t createDateTime_length;
        cass_value_get_string(value, &createDateTime, &createDateTime_length);

        value = cass_row_get_column_by_name(row, "author");
        const char* author;
        size_t author_length;
        cass_value_get_string(value, &author, &author_length);

        value = cass_row_get_column_by_name(row, "optionid");
        int optionid;
        cass_value_get_int32(value, &optionid);

        Vote vote(std::string(id, id_length));
        vote.author = std::string(author, author_length);
        vote.optionId = optionid;

        result.push_back(vote);
    }

    cass_iterator_free(iter);
    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::vote_get(std::string const& id, std::vector<Vote>& result, const char **message) {
    CassStatement* statement = select_vote_query(id);
    const CassResult* query_result = NULL;
    int statusCode = perform_query(statement, &query_result, message);

    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    const CassRow* row = cass_result_first_row(query_result);
    if (row) {
        const CassValue* value = cass_row_get_column_by_name(row, "id");
        const char* id;
        size_t id_length;
        cass_value_get_string(value, &id, &id_length);

        value = cass_row_get_column_by_name(row, "creationDateTime");
        const char* createDateTime;
        size_t createDateTime_length;
        cass_value_get_string(value, &createDateTime, &createDateTime_length);

        value = cass_row_get_column_by_name(row, "author");
        const char* author;
        size_t author_length;
        cass_value_get_string(value, &author, &author_length);

        value = cass_row_get_column_by_name(row, "optionid");
        int optionid;
        cass_value_get_int32(value, &optionid);

        Vote vote(std::string(id, id_length));
        vote.author = std::string(author, author_length);
        vote.optionId = optionid;

        result.push_back(vote);
    }

    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::poll_new(Poll const& poll, const char **message) {
    const CassResult* result = NULL;
    int statusCode = perform_query(insert_poll_query(poll), &result, message);
    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    cass_result_free(result);

    std::vector<CassStatement*> statements;
    statements.push_back(update_poll_votes_query(poll.getId(), 0));
    for (auto it = poll.options.begin(); it != poll.options.end(); ++it) {
        statements.push_back(update_option_votes_query(poll.getId(), it->id, 0));
    }

    statusCode = perform_batch_query(CASS_BATCH_TYPE_COUNTER, statements, message);

    return statusCode;
}

int StorageClient::vote_new(std::string const& pollid, Vote const &vote, const char** message) {
    const CassResult* query_result = NULL;
    int statusCode = perform_query(insert_vote_query(pollid, vote), &query_result, message);
    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    cass_result_free(query_result);

    std::vector<CassStatement*> statements;
    statements.push_back(update_option_votes_query(pollid, vote.optionId, 1));
    statements.push_back(update_poll_votes_query(pollid, 1));

    return perform_batch_query(CASS_BATCH_TYPE_COUNTER, statements, message);
}

int StorageClient::vote_update(std::string const& pollid, Vote const &vote, const char **message) {
    const CassResult* query_result = NULL;
    int statusCode = perform_query(select_vote_query(vote.getId()), &query_result, message);
    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    const CassRow* row = cass_result_first_row(query_result);

    cass_result_free(query_result);

    if (row) {
        const CassValue* value = cass_row_get_column_by_name(row, "optionid");
        int optionid;
        cass_value_get_int32(value, &optionid);

        statusCode = perform_query(update_vote_query(vote), &query_result, message);
        if (statusCode != QUERY_SUCCESS) {
            return statusCode;
        }

        cass_result_free(query_result);

        std::vector<CassStatement*> statements;
        statements.push_back(update_option_votes_query(pollid, optionid, -1));
        statements.push_back(update_option_votes_query(pollid, vote.optionId, 1));
        statusCode = perform_batch_query(CASS_BATCH_TYPE_COUNTER, statements, message);
    }

    return statusCode;
}

int StorageClient::poll_delete(std::string const& id, const char** message) {
    std::vector<CassStatement*> statements;
    statements.push_back(delete_votes_query(id));
    statements.push_back(delete_poll_query(id));

    int statusCode = perform_batch_query(CASS_BATCH_TYPE_UNLOGGED, statements, message);
    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    statements.clear();

    const CassResult* result = NULL;
    statusCode = perform_query(select_poll_option_votes_query(id), &result, message);
    if (statusCode == QUERY_SUCCESS) {
        CassIterator* it = cass_iterator_from_result(result);
        while (cass_iterator_next(it)) {
            const CassRow* optrow = cass_iterator_get_row(it);
            const CassValue* val = cass_row_get_column_by_name(optrow, "optionId");
            int optionid;
            cass_value_get_int32(val, &optionid);

            val = cass_row_get_column_by_name(optrow, "votes");
            long long votes;
            cass_value_get_int64(val, &votes);

            statements.push_back(update_option_votes_query(id, optionid, -votes));
        }
        cass_iterator_free(it);
        cass_result_free(result);
    }

    statusCode = perform_query(select_poll_votes_query(id), &result, message);
    if (statusCode == QUERY_SUCCESS) {
        const CassRow* row = cass_result_first_row(result);
        if (row) {
            const CassValue* val = cass_row_get_column_by_name(row, "totalVotes");
            long long totalVotes;
            cass_value_get_int64(val, &totalVotes);
            statements.push_back(update_poll_votes_query(id, -totalVotes));
        }

        cass_result_free(result);
    }

    if (!statements.empty()) {
        statusCode = perform_batch_query(CASS_BATCH_TYPE_COUNTER, statements, message);
    }

    return  statusCode;
}

int StorageClient::vote_delete(std::string const& pollid, Vote const& vote, const char **message) {
    const CassResult* result = NULL;
    int statusCode = perform_query(delete_vote_query(vote.getId()), &result, message);
    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    std::vector<CassStatement*> statements;
    statements.push_back(update_poll_votes_query(pollid, -1));
    statements.push_back(update_option_votes_query(pollid, vote.optionId, -1));

    return perform_batch_query(CASS_BATCH_TYPE_COUNTER, statements, message);
}
