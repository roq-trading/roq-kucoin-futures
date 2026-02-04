.. _roq-kucoin-futures:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


roq-kucoin-futures
==================


.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-kucoin-futures

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-kucoin-futures


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |cross-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |cross-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        -
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        -
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.

   |footnote-1| The exchange protocol does not support streaming updates for reference data and market status.


Using
-----

.. code-block:: shell

   $ roq-bitget [FLAGS]


.. _roq-kucoin-futures-flags:

Flags
-----

.. code-block:: shell

   $ roq-kucoin-futures --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: MBP

   .. include:: flags/mbp.rstinc

.. tab:: Request

   .. include:: flags/request.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
------------

.. tab:: Prod

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kucoin-futures/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell

.. tab:: Test

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kucoin-futures/flags/test/flags.cfg

   .. include:: flags/test/flags.cfg
     :code: shell

.. tab:: Colo1

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kucoin-futures/flags/colo1/flags.cfg

   .. include:: flags/colo1/flags.cfg
     :code: shell

.. tab:: Colo2

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kucoin-futures/flags/colo2/flags.cfg

   .. include:: flags/colo2/flags.cfg
     :code: shell

.. tab:: Colo3

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-kucoin-futures/flags/colo3/flags.cfg

   .. include:: flags/colo3/flags.cfg
     :code: shell


Configuration
-------------

.. code-block:: shell

   $ --config_file $CONDA_PREFIX/share/roq-kucoin-futures/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: TradingStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Message
       - Field
       - Value
       -
       -

     * - :code:`Contracts`
       - :code:`status`
       - :code:`Open`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN <roq::TradingStatus::OPEN>`


.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       -
       -

     * - :code:`mark.index.price`
       - :code:`markPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`SETTLEMENT_PRICE <roq::StatisticsType::SETTLEMENT_PRICE>`

     * - :code:`mark.index.price`
       - :code:`indexPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`INDEX_VALUE <roq::StatisticsType::INDEX_VALUE>`

     * - :code:`funding.rate`
       - :code:`fundingRate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`funding.begin`
       - :code:`fundingRate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE_PREDICTION <roq::StatisticsType::FUNDING_RATE_PREDICTION>`

     * - :code:`snapshot.24h`
       - :code:`highPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`HIGHEST_TRADED_PRICE <roq::StatisticsType::HIGHEST_TRADED_PRICE>`

     * - :code:`snapshot.24h`
       - :code:`lowPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`LOWEST_TRADED_PRICE <roq::StatisticsType::LOWEST_TRADED_PRICE>`

     * - :code:`snapshot.24h`
       - :code:`volume`
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOLUME <roq::StatisticsType::TRADE_VOLUME>`



Order Management
----------------


Inbound
~~~~~~~

.. tab:: OrderType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`market`
       - |right-double-arrow|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`

     * - :code:`limit`
       - |right-double-arrow|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`


.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`GTC`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`


.. tab:: OrderStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left
     :stub-columns: 1

     * -
       - :code:`type`
       - :code:`remainSize`
       - :code:`status`
       -
       -

     * - |cross-mark|
       - :code:`open`
       -
       -
       -
       -

     * -
       - :code:`match`
       - :code:`== 0`
       -
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - |cross-mark|
       - :code:`update`
       -
       -
       -
       -

     * - |cross-mark|
       - :code:`filled`
       -
       -
       -
       -

     * -
       - :code:`canceled`
       -
       -
       - |right-double-arrow|
       - :cpp:enumerator:`CANCELED <roq::OrderStatus::CANCELED>`

     * -
       -
       -
       - :code:`open`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * -
       -
       -
       - :code:`match`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * -
       -
       -
       - :code:`done`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left
     :stub-columns: 1

     * -
       - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`type`
       - :code:`price`
       - :code:`reduceOnly`

     * - |check-mark|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`market`
       - |cross-mark|
       -

     * - |cross-mark|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       -
       -
       -
       -

     * - |check-mark|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`limit`
       - |cross-mark|
       -

     * - |cross-mark|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       -
       -
       -
       -


.. tab:: ModifyOrder

   TBD


.. tab:: CancelOrder

   TBD


.. tab:: CancelAllOrders

   TBD




Comments
--------

* The default margin mode may have to be configured (flag) if not specified when creating orders.
  This is due to Kucoin **always** defaulting to the :code:`isolated` margin mode.

* No support for stop orders.
  This is due to Roq not currently supporting a stop "direction".

* Downloaded fills don't have the :code:`clientOid` field.
  The implication is that we can't correctly persist historical fills.


References
----------


Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


Exchange
~~~~~~~~

* `Website <https://www.kucoin.com/futures>`__
* `Documentation <https://www.kucoin.com/docs-new/introduction>`__
