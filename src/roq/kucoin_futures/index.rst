.. _roq-kucoin-futures:

.. |checkmark| unicode:: U+2713

roq-kucoin-futures
==================

.. important::
  This gateway needs sponsorship to complete certain features.

Links
-----

* `Website <https://futures.kucoin.com/>`__
* `Support <https://support.kucoin.plus/hc/en-us/categories/4403571259289-Futures-Trading?lang=en_US>`__
* `API <https://docs.kucoin.com/futures/#general>`__


Purpose
-------

* Maintain network connectivity with the KuCoin Futures exchange
* Route exchange updates to connected clients
* Route client requests to the relevant exchange accounts
* Stream all messages to an event-log


Overview
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto

      * - Spot
        -
      * - Futures
        - |checkmark|
      * - Options
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto

      * - Reference Data
        - |checkmark|
      * - Market Status
        - |checkmark|
      * - Top of Book
        - |checkmark|
      * - Market by Price (L2)
        - |checkmark|
      * - Market by Order (L3)
        -
      * - Trade Summary
        - |checkmark|
      * - Statistics
        - |checkmark|

  .. grid-item-card::  Order Management

    .. list-table::
      :widths: auto

      * - Create
        - |checkmark|
      * - Modify
        -
      * - Cancel
        - |checkmark|
      * - Cancel All
        - |checkmark|
      * - Auto Cancellation
        -

  .. grid-item-card::  Account Management

    .. list-table::
      :widths: auto

      * - Positions
        - |checkmark|
      * - Funds
        - |checkmark|

* Data center located in ... (to be confirmed)
* No test environment


Conda
-----

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Install

  .. code-block:: bash

    $ mamba install \
      --channel https://roq-trading.com/conda/stable \
      roq-kucoin-futures

.. tab:: Configure

  .. code-block:: bash

    $ cp $CONDA_PREFIX/share/roq-kucoin-futures/config.toml $CONFIG_FILE_PATH

    # Then modify $CONFIG_FILE_PATH to match your specific configuration

.. tab:: Run

  .. code-block:: bash

    $ roq-kucoin-futures \
          --name "kucoin-futures" \
          --config_file "$CONFIG_FILE_PATH" \
          --client_listen_address "$UNIX_SOCKET_PATH" \
          --service_listen_address "$TCP_LISTEN_PORT" \
          --flagfile "$FLAG_FILE"


Config
------

* :ref:`Common Config <gateway-config>`


Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Common Flags <gateway-flags>`

.. code-block:: bash

   $ roq-kucoin-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: Common

   .. include:: flags/common.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc


Environments
------------

.. code-block:: bash

  $ $CONDA_PREFIX/share/roq-kucoin-futures/flags

.. tab:: Prod

   .. include:: flags/prod/flags.cfg
     :code: ini

.. tab:: Test

   .. include:: flags/test/flags.cfg
     :code: ini


Market Data
-----------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      -  MarketData
      - /contract/instrument
      -

    * - :cpp:class:`roq::MarketStatus`
      - MarketData
      - /contract/instrument
      -

    * - :cpp:class:`roq::TopOfBook`
      - MarketData
      - /contractMarket/tickerV2 or /contractMarket/ticker
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - MarketData
      - /contractMarket/level2
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      - MarketData
      - /contractMarket/execution
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      - MarketData
      - /contract/announcement
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::ReferenceData`
      - Rest
      - /api/v1/contracts/active
      -

    * - :cpp:class:`roq::MarketStatus`
      - Rest
      - /api/v1/contracts/active
      -

    * - :cpp:class:`roq::TopOfBook`
      -
      -
      -

    * - :cpp:class:`roq::MarketByPriceUpdate`
      - Rest
      - /api/v1/level2/snapshot
      -

    * - :cpp:class:`roq::MarketByOrderUpdate`
      -
      -
      -

    * - :cpp:class:`roq::TradeSummary`
      -
      -
      -

    * - :cpp:class:`roq::StatisticsUpdate`
      -
      -
      -


Statistics
~~~~~~~~~~

.. list-table::
  :header-rows: 1
  :widths: auto

  * - Type
    - Comments

  * - :cpp:class:`SETTLEMENT_PRICE`
    - mark_index_price.mark_price

  * - :cpp:class:`INDEX_VALUE`
    - mark_index_price.index_value

  * - :cpp:class:`FUNDING_RATE`
    - funding_rate.funding_rate

  * - :cpp:class:`FUNDING_RATE_PREDICTION`
    - funding_begin.funding_rate

  * - :cpp:class:`TRADE_VOLUME`
    - snapshot_24h.volume


Order Management
----------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - DropCopy
      - /contractMarket/tradeOrders
      -

    * - :cpp:class:`roq::TradeUpdate`
      -
      -
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderUpdate`
      - OrderEntry
      - GET /api/v1/orders
      - It's only possible to poll?

    * - :cpp:class:`roq::TradeUpdate`
      - OrderEntry
      - GET /api/v1/fills
      - It's only possible to poll?

.. tab:: Request

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::CreateOrder`
      - OrderEntry
      - POST /api/v1/orders
      -

    * - :cpp:class:`roq::ModifyOrder`
      -
      -
      -

    * - :cpp:class:`roq::CancelOrder`
      - OrderEntry
      - DELETE /api/v1/orders/{order-id}
      -

    * - :cpp:class:`roq::CancelAllOrders`
      - OrderEntry
      - DELETE /api/v1/orders
      -

.. tab:: Response

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::OrderAck`
      -
      -
      -


Order Types
~~~~~~~~~~~

TBD


Time in Force
~~~~~~~~~~~~~

TBD


Position Effect
~~~~~~~~~~~~~~~

TBD


Execution Instructions
~~~~~~~~~~~~~~~~~~~~~~

TBD


Account Management
------------------

.. tab:: Live

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - DropCopy
      - /contract/position:{symbol}
      -

    * - :cpp:class:`roq::FundsUpdate`
      - DropCopy
      - /contractAccount/wallet
      -

.. tab:: Download

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Event
      - Stream
      - Messages
      - Comments

    * - :cpp:class:`roq::PositionUpdate`
      - OrderEntry
      - GET /api/v1/position
      -

    * - :cpp:class:`roq::FundsUpdate`
      -
      -
      -


Streams
-------

.. tab:: Rest

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - REST
      - Primary purpose

        * download reference data and L2 snapshot

.. tab:: MarketData

  .. list-table::
    :header-rows: 1
    :widths: auto

    * - Type
      - Comments

    * - WebSocket
      - Primary purpose

        * live market data
        * reference data and market status

        Each connection

        * supports a slice of the symbols


Constraints
-----------

* It does not appear to be possible to subscribe all symbols more than once per IP address.


Comments
--------

* Orders can **NOT** be canceled by client order id.

* Fills are not communicated in real-time (only indirectly through order updates).

* There are no options to instruct the exchange to auto-cancel orders on disconnect.

* API v2 has been announced.
  As of 2022-07-01, it is unclear when this new API will be released to production.
  However, it sounds like it will be a change with no transition period with support 
  for both API's.

  * `2022-05-31 <https://www.kucoin.com/news/en-kucoin-futures-api-update-notice/>`__
  * `2022-06-24 <https://www.kucoin.com/news/hk-the-launch-of-the-new-paper-trading-system-for-KuCoin-Futures-on-June-24-2022/>`__
  * `docs <https://docs.kucoin.com/futures/new>`__
