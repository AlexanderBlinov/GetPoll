//
// Created by ubuntu on 25.12.16.
//

#include "StorageClient.h"
#include "Query.h"

int const QUERY_SUCCESS = 0;
int const QUERY_FAILURE = 1;

StorageClient::StorageClient(const char* hosts) {
    this->hosts = hosts;
}

StorageClient::~StorageClient() {
    cass_cluster_free(this->cluster);
    cass_session_free(this->session);
    delete[] this->hosts;
}

CassFuture *StorageClient::connect(const char** message) {
    CassFuture* connect_future;
    connect_future = cass_session_connect(this->getSession(), this->getCluster());
    if (cass_future_error_code(connect_future) == CASS_OK) {
        return connect_future;
    }

    cass_future_error_message(connect_future, message, NULL);

    return NULL;
}

CassCluster *StorageClient::getCluster() {
    if (this->cluster == NULL) {
        this->cluster = cass_cluster_new();
        cass_cluster_set_contact_points(this->cluster, this->hosts);
    }
    return this->cluster;
}

CassSession *StorageClient::getSession() {
    if (this->session == NULL) {
        this->session = cass_session_new();
    }
    return this->session;
}

int StorageClient::perform_query(const char* query, const CassResult** result, const char** message) {
    int statusCode = QUERY_FAILURE;

    CassFuture* connect_future = this->connect(message);
    if (connect_future) {
        CassFuture* close_future = NULL;

        CassStatement* statement = cass_statement_new(query, 0);

        CassFuture* result_future = cass_session_execute(this->session, statement);
        cass_statement_free(statement);

        if (cass_future_error_code(result_future) == CASS_OK) {
            *result = cass_future_get_result(result_future);
            statusCode = QUERY_SUCCESS;
        } else {
            cass_future_error_message(result_future, message, NULL);
        }

        cass_future_free(result_future);
        cass_future_free(connect_future);

        close_future = cass_session_close(this->session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
    }

    return statusCode;
}

int StorageClient::perform_batch_query(std::vector<std::string>& queries, const char **message) {
    int statusCode = QUERY_FAILURE;

    CassFuture* connect_future = this->connect(message);
    if (connect_future) {
        CassFuture* close_future = NULL;

        CassBatch* batch = cass_batch_new(CASS_BATCH_TYPE_UNLOGGED);

        for (std::vector<std::string>::iterator it = queries.begin(); it != queries.end(); ++it) {
            CassStatement* statement = cass_statement_new((*it).c_str(), 0);
            cass_batch_add_statement(batch, statement);
            cass_statement_free(statement);
        }

        CassFuture* batch_future = cass_session_execute_batch(session, batch);

        cass_batch_free(batch);

        if (cass_future_error_code(batch_future) == CASS_OK) {
            statusCode = QUERY_SUCCESS;
        } else {
            cass_future_error_message(batch_future, message, NULL);
        }

        cass_future_free(batch_future);
        cass_future_free(connect_future);

        close_future = cass_session_close(this->session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
    }

    return statusCode;
}

int StorageClient::polls_get(std::string& creationDateTime, std::vector<Poll>& result, const char** message) {
    const CassResult* query_result = NULL;

    int limit = 10;
    const char* query = select_polls_query(limit, creationDateTime.c_str());

    delete[] query;

    int statusCode = this->perform_query(query, &query_result, message);
    if (statusCode != QUERY_SUCCESS) {
        return  statusCode;
    }

    CassIterator* iter = cass_iterator_from_result(query_result);
    while (cass_iterator_next(iter)) {
        const CassRow* row = cass_iterator_get_row(iter);
        const CassValue* value = cass_row_get_column_by_name(row, "id");

        const char* id;
        size_t id_length;
        cass_value_get_string(value, &id, &id_length);

        const char* creationDT;
        size_t creationDateTime_length;
        value = cass_row_get_column_by_name(row, "creationDateTime");
        cass_value_get_string(value, &creationDT, &creationDateTime_length);

        const char* name;
        size_t name_length;
        value = cass_row_get_column_by_name(row, "name");
        cass_value_get_string(value, &name, &name_length);

        Poll poll;
        poll.id = std::string(id, id_length);
        poll.creationDateTime = std::string(creationDT, creationDateTime_length);
        poll.name = std::string(name, name_length);

        result.push_back(poll);
    }

    cass_iterator_free(iter);
    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::poll_get(std::string& id, std::vector<Poll> &result, const char** message) {
    const CassResult* query_result = NULL;

    const char* query = select_poll_query(id.c_str());

    int statusCode = this->perform_query(query, &query_result, message);

    delete[] query;

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

        Poll poll;
        poll.id = id;
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

        query = select_poll_votes_query(id.c_str());
        statusCode = this->perform_query(query, &query_result, message);

        delete[] query;

        if (statusCode != QUERY_SUCCESS) {
            return  statusCode;
        }

        row = cass_result_first_row(query_result);
        cass_result_free(query_result);

        value = cass_row_get_column_by_name(row, "totalVotes");
        cass_value_get_int32(value, &(poll.totalVotes));

        query = select_poll_option_votes_query(id.c_str());
        statusCode = this->perform_query(query, &query_result, message);

        delete[] query;

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
            for (std::vector<PollOption>::iterator it = poll.options.begin(); it != poll.options.end(); ++it) {
                if (it->id == optionid) {
                    val = cass_row_get_column_by_name(row, "votes");
                    int votes;
                    cass_value_get_int32(val, &votes);
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

int StorageClient::votes_get(std::string& pollid, std::vector<Vote>& result, const char** message) {
    const char* query = select_votes_query(pollid.c_str());
    const CassResult* query_result = NULL;
    int statusCode = this->perform_query(query, &query_result, message);

    delete[] query;

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

        Vote vote;
        vote.id = std::string(id, id_length);
        vote.author = std::string(author, author_length);
        vote.optionId = optionid;

        result.push_back(vote);
    }

    cass_iterator_free(iter);
    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::vote_get(std::string& id, std::vector<Vote> &result, const char **message) {
    const char* query = select_vote_query(id.c_str());
    const CassResult* query_result = NULL;
    int statusCode = this->perform_query(query, &query_result, message);

    delete[] query;

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

        Vote vote;
        vote.id = std::string(id, id_length);
        vote.author = std::string(author, author_length);
        vote.optionId = optionid;

        result.push_back(vote);
    }

    cass_result_free(query_result);

    return statusCode;
}

int StorageClient::poll_new(Poll const &poll, const char **message) {
    std::vector<std::string> queries;
    queries.push_back(std::string(insert_poll_query(poll)));
    queries.push_back(std::string(insert_poll_votes_query(poll.id.c_str())));
    for (std::vector<PollOption>::const_iterator it = poll.options.begin(); it != poll.options.end(); ++it) {
        queries.push_back(std::string(insert_option_votes_query(poll.id.c_str(), it->id)));
    }

    return this->perform_batch_query(queries, message);
}

int StorageClient::vote_new(std::string& pollid, Vote const &vote, const char** message) {
    std::vector<std::string> queries;
    queries.push_back(std::string(insert_vote_query(pollid.c_str(), vote)));
    queries.push_back(std::string(update_option_votes_query(pollid.c_str(), vote.optionId, 1)));
    queries.push_back(std::string(update_poll_votes_query(pollid.c_str(), 1)));

    return this->perform_batch_query(queries, message);
}

int StorageClient::vote_update(std::string& pollid, Vote const &vote, const char **message) {
    const char* query = select_vote_query(vote.id.c_str());
    const CassResult* query_result = NULL;
    int statusCode = this->perform_query(query, &query_result, message);

    delete[] query;

    if (statusCode != QUERY_SUCCESS) {
        return statusCode;
    }

    const CassRow* row = cass_result_first_row(query_result);

    cass_result_free(query_result);

    if (row) {
        const CassValue* value = cass_row_get_column_by_name(row, "optionid");
        int optionid;
        cass_value_get_int32(value, &optionid);

        std::vector<std::string> queries;
        queries.push_back(std::string(update_poll_votes_query(pollid.c_str(), -1)));
        queries.push_back(std::string(update_option_votes_query(pollid.c_str(), optionid, -1)));
        queries.push_back(std::string(update_vote_query(vote)));

        statusCode = this->perform_batch_query(queries, message);
    }

    return statusCode;
}

int StorageClient::poll_delete(std::string& id, const char** message) {
    std::vector<std::string> queries;
    queries.push_back(std::string(delete_option_votes_query(id.c_str())));
    queries.push_back(std::string(delete_poll_votes_query(id.c_str())));
    queries.push_back(std::string(delete_votes_query(id.c_str())));
    queries.push_back(std::string(delete_poll_query(id.c_str())));

    return this->perform_batch_query(queries, message);
}

int StorageClient::vote_delete(std::string& pollid, Vote const& vote, const char **message) {
    std::vector<std::string> queries;
    queries.push_back(std::string(update_poll_votes_query(pollid.c_str(), -1)));
    queries.push_back(std::string(update_option_votes_query(pollid.c_str(), vote.optionId, -1)));
    queries.push_back(std::string(delete_vote_query(vote.id.c_str())));

    return this->perform_batch_query(queries, message);
}
