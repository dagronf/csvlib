//
//  Document.swift
//  Tabula Rasa
//
//  Created by Darren Ford on 13/10/18.
//  Copyright © 2018 Darren Ford. All rights reserved.
//

import Cocoa

class Document: NSDocument {
	private var tableData: TabulaRasaData?

	@IBOutlet var tableViewController: TabulaRasaTableViewController!
	@IBOutlet var containingView: NSView!
	@IBOutlet weak var loadingProgress: NSProgressIndicator!
	@IBOutlet weak var loadingLabel: NSTextField!
	@IBOutlet weak var searchResultCountField: NSTextField!
	
	@IBOutlet weak var searchProgress: NSProgressIndicator!
	private let indexer = DFSearchIndex.Memory.Create()!

	private var isClosing = false
	
	override init() {
		super.init()
		// Add your subclass-specific initialization here.
	}

	override class var autosavesInPlace: Bool {
		return true
	}

	override var windowNibName: NSNib.Name? {
		// Returns the nib file name of the document
		// If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this property and override -makeWindowControllers instead.
		return NSNib.Name("Document")
	}

//	override func data(ofType typeName: String) throws -> Data {
//		// Insert code here to write your document to data of the specified type, throwing an error in case of failure.
//		// Alternatively, you could remove this method and override fileWrapper(ofType:), write(to:ofType:), or write(to:ofType:for:originalContentsURL:) instead.
//		throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
//	}

	override func read(from url: URL, ofType typeName: String) throws {
		if typeName == "TSV Document Type" {
			self.tableData = TabulaRasaData(url: url, type: .tsv)
		}
		else {
			self.tableData = TabulaRasaData(url: url, type: .csv)
		}
	}

	override func close() {
		self.isClosing = true
		if let tableData = self.tableData {
			tableData.cancel()
		}
		super.close()
	}

	@IBAction func performSearch(_ sender: NSSearchField) {
		let str = sender.stringValue
		let results = self.find(text: str)
		self.searchResultCountField.stringValue = "\(results.count) result(s)"
		self.tableViewController.highlightSearchResults(results: results)
	}

	@IBAction func changeMatchedSearch(_ sender: NSSegmentedControl) {
		let forward = sender.selectedSegment == 1
		self.tableViewController.moveToNextMatch(forwards: forward)
	}

	private func fit(childView: NSView, parentView: NSView) {
		childView.translatesAutoresizingMaskIntoConstraints = false
		childView.topAnchor.constraint(equalTo: parentView.topAnchor).isActive = true
		childView.leadingAnchor.constraint(equalTo: parentView.leadingAnchor).isActive = true
		childView.trailingAnchor.constraint(equalTo: parentView.trailingAnchor).isActive = true
		childView.bottomAnchor.constraint(equalTo: parentView.bottomAnchor).isActive = true
	}

	override func windowControllerDidLoadNib(_: NSWindowController) {
		self.tableViewController.loadView()

		self.searchProgress.startAnimation(self)

		self.containingView.addSubview(self.tableViewController.view)
		self.fit(childView: self.tableViewController.view, parentView: self.containingView)

		_ = self.tableData?.loadAsync(fraction: { [weak self] (progress) in
			self?.loadingProgress.doubleValue = Double(progress)
		}, completion: { [weak self] in
			DispatchQueue.main.async {
				self?.dataLoaded()
			}
		})
	}

	private func dataLoaded() {
		self.tableViewController.update(source: self.tableData!)
		self.loadingLabel.isHidden = true
		self.loadingProgress.isHidden = true

		self.createIndex()
	}

	private func generateIndex() {
		DispatchQueue.global(qos: .userInitiated).async { [weak self] in

			var count = 0

			for row in self!.tableData!.rawData.enumerated() {
				if self!.isClosing {
					return
				}
				for field in row.element.enumerated() {
					if self!.isClosing {
						return
					}
					_ = self?.indexer.add(textURL: "field://\(row.offset):\(field.offset)", text: field.element)
				}
				count += 1
				DispatchQueue.main.async {
					self?.searchProgress.doubleValue = Double(count)
				}

				// Only flush to the index every 1000 records so
				if row.offset % 1000 == 0 {
					self?.indexer.flush()
				}
			}

			self?.indexer.flush()
			self?.indexer.compact()
			self?.searchProgress.isHidden = true
		}
	}

	private func createIndex() {

		let total = self.tableData!.rawData.count
		self.searchProgress.minValue = 0
		self.searchProgress.maxValue = Double(total)
		self.generateIndex()
	}

	public func find(text: String) -> [DFSearchIndex.SearchResult] {
		return self.indexer.search(text)
	}


}
