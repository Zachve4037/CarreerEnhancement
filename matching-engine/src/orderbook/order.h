#ifndef CPPFIRST_ORDER_H
#define CPPFIRST_ORDER_H
#include <cstdint>
#include <optional>

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint64_t;
using Sequence = std::uint64_t;

enum class Side { Buy, Sell };

enum class OrderStatus {
  New,
  Open,
  PartiallyFilled,
  Filled,
  Cancelled,
  Rejected
};

struct Order {
  OrderId id;
  Side side;
  Price price;
  Quantity quantity;
  Quantity filled_quantity;
  Sequence sequence;
  OrderStatus status;

  Order(OrderId order_id, Side order_side, Price order_price, Quantity order_quantity,
        Sequence order_sequence)
      : id(order_id), side(order_side), price(order_price), quantity(order_quantity),
        filled_quantity(0), sequence(order_sequence), status(OrderStatus::New) {}

  [[nodiscard]]
  Quantity remaining_quantity() const noexcept {
    return quantity - filled_quantity;
  }


};

struct NewOrder {
  OrderId id;
  Side side;
  Price price;
  Quantity quantity;
};

struct CancelOrder {
  OrderId id;
};

enum class ValidationError {
  InvalidOrderId,
  InvalidSide,
  InvalidPrice,
  InvalidQuantity
};

[[nodiscard]]
inline std::optional<ValidationError> validate(const NewOrder& command) {
  if (command.id <= 0) {
    return ValidationError::InvalidOrderId;
  }
  if (command.side != Side::Buy && command.side != Side::Sell) {
    return ValidationError::InvalidSide;
  }
  if (command.price <= 0) {
    return ValidationError::InvalidPrice;
  }
  if (command.quantity == 0) {
    return ValidationError::InvalidQuantity;
  }
  return std::nullopt;
}

[[nodiscard]]
inline std::optional<ValidationError>
validate(const CancelOrder& command) {
  if (command.id == 0) {
    return ValidationError::InvalidOrderId;
  }

  return std::nullopt;
}

#endif // CPPFIRST_ORDER_H
