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
		if let tableData = self.tableData {
			tableData.cancel()
		}
		super.close()
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

		self.containingView.addSubview(self.tableViewController.view)
		self.fit(childView: self.tableViewController.view, parentView: self.containingView)

		_ = self.tableData?.loadAsync(fraction: { [weak self] (progress) in
			self?.loadingProgress.doubleValue = Double(progress)
		}, completion: { [weak self] in
			self?.dataLoaded()
		})
	}

	func dataLoaded() {
		self.tableViewController.update(source: self.tableData!)
	}
}
