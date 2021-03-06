/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
#include <graphene/chain/protocol/wire_out_with_fee.hpp>
#include <graphene/chain/wire_out_with_fee_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

const wire_out_with_fee_holder_object& database_fixture::wire_out_with_fee(account_id_type account_id, asset asset_to_wire,
                                                                           const string& currency_of_choice,
                                                                           const string& to_address, const string& memo)
{ try {

  wire_out_with_fee_operation op;
  op.account = account_id;
  op.asset_to_wire = asset_to_wire;
  op.currency_of_choice = currency_of_choice;
  op.to_address = to_address;
  op.memo = memo;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();

  return db.get<wire_out_with_fee_holder_object>(ptx.operation_results[0].get<object_id_type>());

} FC_LOG_AND_RETHROW() }

vector<wire_out_with_fee_holder_object> database_fixture::get_wire_out_with_fee_holders(account_id_type account_id,
                                                                      const flat_set<asset_id_type>& asset_ids) const
{
   vector<wire_out_with_fee_holder_object> result;
   const auto& idx = db.get_index_type<wire_out_with_fee_holder_index>().indices().get<by_account_asset>();
   if ( asset_ids.empty() )
   {
      // if the caller passes in an empty list of asset ID's, return holders for all assets the account owns
      for ( auto it = idx.find(boost::make_tuple(account_id)); it != std::end(idx) && it->account == account_id; ++it )
        result.emplace_back(*it);
   }
   else
   {
      for ( auto asset_id : asset_ids )
      {
        for ( auto it = idx.find(boost::make_tuple(account_id, asset_id));
              it != std::end(idx) && it->account == account_id;
              ++it
            )
          result.emplace_back(*it);
      }
   }
   return result;
}

void database_fixture::wire_out_with_fee_complete(wire_out_with_fee_holder_id_type holder_id)
{
  wire_out_with_fee_complete_operation op;

  op.wire_out_handler = get_wire_out_handler_id();
  op.holder_object_id = holder_id;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

void database_fixture::wire_out_with_fee_reject(wire_out_with_fee_holder_id_type holder_id)
{
  wire_out_with_fee_reject_operation op;

  op.wire_out_handler = get_wire_out_handler_id();
  op.holder_object_id = holder_id;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();
}

} }  // namespace graphene::chain
