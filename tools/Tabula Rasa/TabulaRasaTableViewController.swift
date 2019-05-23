//
//  TablulaRasaTableViewController.swift
//  Tabula Rasa
//
//  Created by Darren Ford on 13/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

import Cocoa

private struct SearchResult {
	let row: Int
	let col: Int
}

class TabulaRasaTableViewController: NSViewController, NSTableViewDataSource, NSTableViewDelegate {
	@IBOutlet var results: NSTableView!

	private var searchMatches: [SearchResult] = []
	private var selectedMatch: Int = -1

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

	func highlightSearchResults(results: [DFSearchIndex.SearchResult]) {

		self.selectedMatch = 0
		
		let origRows: [Int] = self.searchMatches.map { $0.row }
		let origCols: [Int] = self.searchMatches.map { $0.col + 1 }
		
		self.searchMatches.removeAll()
		for result in results {
			let row = Int(result.url.host!)!
			let col = Int(result.url.port!)
			self.searchMatches.append(SearchResult(row: row, col: col))
		}
		
		self.searchMatches.sort { (left, right) -> Bool in
			if left.row < right.row {
				return true
			}
			if left.row > right.row {
				return false
			}
			if left.col < right.col {
				return true
			}
			if left.col > right.col {
				return false
			}
			return true
		}
		
		let rows = origRows + self.searchMatches.map({ $0.row })
		let cols = origCols + self.searchMatches.map({ $0.col + 1 })
		
		self.results.reloadData(forRowIndexes: IndexSet(rows), columnIndexes: IndexSet(cols))
		
		if (self.searchMatches.count > 0) {
			NSAnimationContext.runAnimationGroup { (context) in
				context.allowsImplicitAnimation = true;
				self.results.scrollRowToVisible(self.searchMatches[self.selectedMatch].row)
				self.results.scrollColumnToVisible(self.searchMatches[self.selectedMatch].col + 1)
			}
		}
	}

	func moveToNextMatch(forwards: Bool) {

		let previousSelection = self.selectedMatch
		if forwards {
			selectedMatch += 1
			if selectedMatch >= self.searchMatches.count {
				selectedMatch = 0
			}
		}
		else {
			selectedMatch -= 1
			if selectedMatch < 0 {
				selectedMatch = self.searchMatches.count - 1
			}
		}

		if searchMatches.count > 0 {
			self.results.reloadData(forRowIndexes: IndexSet([searchMatches[previousSelection].row, searchMatches[selectedMatch].row]),
									columnIndexes: IndexSet(0..<self.results.tableColumns.count))

			NSAnimationContext.runAnimationGroup { (context) in
				context.allowsImplicitAnimation = true;
				self.results.scrollRowToVisible(self.searchMatches[self.selectedMatch].row)
				self.results.scrollColumnToVisible(self.searchMatches[self.selectedMatch].col + 1)
			}
		}
	}

	func numberOfRows(in _: NSTableView) -> Int {
		guard let rawData = self.dataSource?.rawData else {
			return 0
		}

		return self.searchMatches.count > 0 ? self.searchMatches.count : rawData.count
	}

	func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
		guard let rawData = self.dataSource?.rawData else {
			return nil
		}

		guard let cell = tableView.makeView(withIdentifier: NSUserInterfaceItemIdentifier(rawValue: "basicCell"), owner: nil) as? NSTableCellView else {
			return nil
		}

		guard let column = Int((tableColumn?.identifier.rawValue)!) else {
			assert(false)
			return nil
		}
		cell.wantsLayer = true

		let matches = self.searchMatches.filter ({ $0.row == row && $0.col == column-1 })
		if matches.count > 0 {
			let active = searchMatches[selectedMatch]
			if active.row == row && active.col == column-1 {
				cell.layer?.backgroundColor = NSColor.findHighlightColor.cgColor
				cell.textField?.textColor = NSColor.textBackgroundColor
			}
			else {
				cell.layer?.backgroundColor = NSColor.selectedTextBackgroundColor.cgColor
				cell.textField?.textColor = NSColor.selectedTextColor
			}
		}
		else {
			cell.layer?.backgroundColor = nil
			cell.textField?.textColor = NSColor.textColor
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
}
