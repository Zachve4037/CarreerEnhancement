# Public API DOC
## Commands
* **NEW** - for creating new order
* **BUY** - for changing order side to *BUY* side
* **SELL** - for changing order side to *SELL* side
* **CANCEL** - for cancelling order

## Order manipulation
*NEW 1 BUY 10500 10* - creates new order with id 1 and put it to *BUY* side, wanting to buy with price 10500 and quantity 10
There have to be exactly 5 tokens, when less -> *MissingArgument* / when more -> *UnexpectedArgumennt*

### Table of tokens
| Position | Meaning |
| ------------- |:-------------:|
| 1 | OrderCommand { NEW, CANCEL } - creating or cancelling order|
| 2 | OrderId - must be greater than 0|
| 3 | OrderSide { BUY / SELL } - assigning orders side|
| 4 | OrderPrice - must be greater than 0|
| 5 | OrderQuantity - cannot be equal to 0|