.. zephyr:code-sample:: ble_direction_finding_connectionless_rx
   :name: Direction Finding Periodic Advertising Locator
   :relevant-api: bt_gap bluetooth

   Implement Bluetooth LE Direction Finding CTE Locator functionality.

Overview
********

A simple application demonstrating the Bluetooth LE Direction Finding CTE Locator
functionality by receiving and sampling sending Constant Tone Extension with
periodic advertising PDUs.

Requirements
************

* Nordic nRF SoC based board with Direction Finding support (example boards:
  :zephyr:board:`nrf52833dk`, :zephyr:board:`nrf5340dk`)
* Antenna matrix for AoA (optional)

Check your SoC's product specification for Direction Finding support if you are
unsure.

Building and Running
********************

By default the application supports Angle of Arrival and Angle of Departure mode.

To use Angle of Departure mode only, build this application as follows,
changing ``nrf52833dk/nrf52833`` as needed for your board:

.. zephyr-app-commands::
   :zephyr-app: samples/bluetooth/direction_finding_connectionless_rx
   :host-os: unix
   :board: nrf52833dk/nrf52833
   :gen-args: -DEXTRA_CONF_FILE=overlay-aod.conf
   :goals: build flash
   :compact:

To run the application on nRF5340DK, a Bluetooth controller application must
also run on the network core. The :zephyr:code-sample:`bluetooth_hci_ipc` sample
application may be used. To build this sample with direction finding support
enabled:

* Copy
  :zephyr_file:`samples/bluetooth/direction_finding_connectionless_rx/boards/nrf52833dk_nrf52833.overlay`
  to a new file,
  :file:`samples/bluetooth/hci_ipc/boards/nrf5340dk_nrf5340_cpunet.overlay`.
* Make sure the same GPIO pins are assigned to Direction Finding Extension in file
  :zephyr_file:`samples/bluetooth/direction_finding_connectionless_rx/boards/nrf5340dk_nrf5340_cpuapp.overlay`.
  as those in the created file :file:`samples/bluetooth/hci_ipc/boards/nrf5340dk_nrf5340_cpunet.overlay`.
* Copy
  :zephyr_file:`samples/bluetooth/direction_finding_connectionless_rx/boards/nrf52833dk_nrf52833.conf`
  to a new file,
  :file:`samples/bluetooth/hci_ipc/boards/nrf5340dk_nrf5340_cpunet.conf`. Add
  the line ``CONFIG_BT_EXT_ADV=y`` to enable extended size of
  :kconfig:option:`CONFIG_BT_BUF_CMD_TX_SIZE` to support the LE Set Extended Advertising
  Data command.

Antenna matrix configuration
****************************

To use this sample with Angle of Departure enabled on Nordic SoCs, additional
configuration must be provided via :ref:`devicetree <dt-guide>` to enable
control of the antenna array.

An example devicetree overlay is in
:zephyr_file:`samples/bluetooth/direction_finding_connectionless_rx/boards/nrf52833dk_nrf52833.overlay`.
You can customize this overlay when building for the same board, or create your
own board-specific overlay in the same directory for a different board. See
:dtcompatible:`nordic,nrf-radio` for documentation on the properties used in
this overlay. See :ref:`set-devicetree-overlays` for information on setting up
and using overlays.

Note that antenna matrix configuration for the nRF5340 SoC is part of the
network core application. When :zephyr:code-sample:`bluetooth_hci_ipc` is used as the
network core application, the antenna matrix configuration should be stored in
the file
:file:`samples/bluetooth/hci_ipc/boards/nrf5340dk_nrf5340_cpunet.overlay`
instead.
