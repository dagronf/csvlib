//
//  TablulaRasaTableViewController.swift
//  Tabula Rasa
//
//  Created by Darren Ford on 13/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

import Cocoa

class TabulaRasaTableViewController: NSViewController, NSTableViewDataSource, NSTableViewDelegate {
	@IBOutlet var results: NSTableView!

	private var dataSource: TabulaRasaData?

	override func viewDidLoad() {
		super.viewDidLoad()
		// Do view setup here.
	}

	func update(source: TabulaRasaData) {
		if self.results == nil {
			self.loadView()
		}

		self.dataSource = source

		for col in self.results.tableColumns.dropFirst() {
			self.results.removeTableColumn(col)
		}

		guard let rawData = self.dataSource?.rawData else {
			return
		}

		// Find the largest column count
		let colCount = rawData.map { $0.count }.max() ?? 0

		for i in 0 ..< colCount {
			let newCol = NSTableColumn()
			newCol.identifier = NSUserInterfaceItemIdentifier(rawValue: String(i + 1))
			newCol.title = "Column \(i + 1)"
			self.results.addTableColumn(newCol)
		}

		self.results.reloadData()
	}

	func numberOfRows(in _: NSTableView) -> Int {
		guard let rawData = self.dataSource?.rawData else {
			return 0
		}
		return rawData.count
	}

	func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
		guard let rawData = self.dataSource?.rawData else {
			return nil
		}

		if let cell = tableView.makeView(withIdentifier: NSUserInterfaceItemIdentifier(rawValue: "basicCell"), owner: nil) as? NSTableCellView {
			guard let column = Int((tableColumn?.identifier.rawValue)!) else {
				assert(false)
				return nil
			}

			if column == 0 {
				cell.textField?.stringValue = "Row \(row + 1)"
			}
			else {
				let rowData = rawData[row]
				if column <= rowData.count {
					cell.textField?.stringValue = rowData[column - 1]
					cell.toolTip = rowData[column - 1]
				}
				else {
					return nil
				}
			}
			return cell
		}
		return nil
	}
}
